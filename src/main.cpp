#include <format>
#include <string>

#include <IniReader.h>
#include <Manager.h>

#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "secrets.h"

#define VERSION "1.0"
#define POSIX_TIMEZONE "CET-1CEST,M3.5.0/2,M10.5.0/3" // Belgium

Manager manager;

static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    wifi_config_t wifi_config ={
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    #pragma GCC diagnostic pop

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    esp_wifi_set_ps(WIFI_PS_NONE);
    esp_wifi_set_max_tx_power(78);
}

void time_init(void)
{
    setenv("TZ", POSIX_TIMEZONE, 1);
    tzset();
    
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

std::string GetFormattedTime()
{
    time_t now;
    time(&now);

    struct tm timeInfo;
    localtime_r(&now, &timeInfo);

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);

    return std::string(buffer);
}

esp_err_t index_handler(httpd_req_t *req)
{
    std::string response = "Domotech CTRL\n\n";

    response.append("Version: ");
    response.append(std::string(VERSION));
    response.append("\n");

    response.append("Free memory: ");
    response.append(std::to_string(esp_get_free_heap_size()));
    response.append("bytes\n");

    wifi_ap_record_t apInfo;
    if (esp_wifi_sta_get_ap_info(&apInfo) == ESP_OK) {
        response.append("Wifi RSSI: ");
        response.append(std::to_string(apInfo.rssi));
        response.append("dBm, quality: ");
        const auto quality = apInfo.rssi <= -100 ? 0 : apInfo.rssi >= -50 ? 100 : 2 * (apInfo.rssi + 100);
        response.append(std::to_string(quality));
        response.append("%\n");
    }

    response.append("Current time: ");
    response.append(GetFormattedTime());

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
}

esp_err_t known_filters_handler(httpd_req_t *req)
{
    const auto ini = manager.GetKnownFiltersIni();
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, ini.c_str());
    return ESP_OK;
}

esp_err_t configuration_handler(httpd_req_t *req)
{
    const auto ini = manager.GetConfigurationIni();
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, ini.c_str());
    return ESP_OK;
}

esp_err_t configuration_clear_handler(httpd_req_t *req)
{
    manager.Clear();
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Configuration cleared");
    return ESP_OK;
}

esp_err_t configuration_rescan_handler(httpd_req_t *req)
{
    const auto result = manager.RescanModules();
    std::string response = "Found " + std::to_string(result.NumberOfDetectedModules) + " module(s)";
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
}

esp_err_t configuration_create_filter_handler(httpd_req_t *req)
{
    char body[256];
    int received = httpd_req_recv(req, body, sizeof(body) - 1);
    if (received <= 0) {
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive body");
        return ESP_FAIL;
    }

    body[received] = '\0'; // Terminate

    std::string id;
    std::string type;
    std::string connections;
    size_t numberOfFiltersCreated = 0;

    IniReader reader;
    reader.OnSection([&](std::string_view section) {
        id.clear();
        type.clear();
        connections.clear();
    });
    reader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value) {
        if (section != "Filter") return;
        if (key == "Id") id = value;
        if (key == "Type") type = value;
        if (key == "Connections") connections = value;

        if (!id.empty() && !type.empty() && !connections.empty()) {
            if (manager.TryCreateFilter(type, id, connections))
                numberOfFiltersCreated++;

            type.clear();
            connections.clear();
        }
    });
    reader.Feed(body, received);
    reader.Finalize();

    std::string response = "Created " + std::to_string(numberOfFiltersCreated) + " filter(s)";
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
}

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t hello_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &hello_uri);

        httpd_uri_t known_filters_uri = {
            .uri       = "/known-filters",
            .method    = HTTP_GET,
            .handler   = known_filters_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &known_filters_uri);

        httpd_uri_t configuration_uri = {
            .uri       = "/configuration",
            .method    = HTTP_GET,
            .handler   = configuration_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &configuration_uri);

        httpd_uri_t configuration_clear_uri = {
            .uri       = "/configuration/clear",
            .method    = HTTP_POST,
            .handler   = configuration_clear_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &configuration_clear_uri);

        httpd_uri_t configuration_rescan_uri = {
            .uri       = "/configuration/rescan",
            .method    = HTTP_POST,
            .handler   = configuration_rescan_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &configuration_rescan_uri);

        httpd_uri_t configuration_create_filter_uri = {
            .uri       = "/configuration/filter",
            .method    = HTTP_POST,
            .handler   = configuration_create_filter_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &configuration_create_filter_uri);
    }

    return server;
}

void ProcessTask(void *arg)
{
    while (true) {
        manager.ProcessNext();
        vTaskDelay(1);
    }

    vTaskDelete(NULL);
}

extern "C" void app_main()
{
    nvs_flash_init();   // Required for Wi-Fi
    wifi_init_sta();    // Connect to Wi-Fi

    manager.Start();

    xTaskCreate(
        ProcessTask,    // Task function
        "Process",      // Name (for debugging)
        4096,           // Stack size in bytes
        NULL,           // Task parameter
        5,              // Task priority (higher = more important)
        NULL            // Optional handle
    );

    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);

    time_init();

    start_webserver();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
