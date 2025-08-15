#include <string>
#include <format>

#include <Manager.h>
#include <IniReader.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "secrets.h"

#define LED_GPIO GPIO_NUM_2

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

    // Wait for connection
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}

esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_sendstr(req, "Hello, World!");
    return ESP_OK;
}

esp_err_t known_filters_handler(httpd_req_t *req)
{
    const auto ini = manager.GetKnownFiltersIni();
    httpd_resp_sendstr(req, ini.c_str());
    return ESP_OK;
}

esp_err_t configuration_handler(httpd_req_t *req)
{
    const auto ini = manager.GetConfigurationIni();
    httpd_resp_sendstr(req, ini.c_str());
    return ESP_OK;
}

esp_err_t configuration_clear_handler(httpd_req_t *req)
{
    manager.Clear();
    httpd_resp_sendstr(req, "Configuration cleared");
    return ESP_OK;
}

esp_err_t configuration_rescan_handler(httpd_req_t *req)
{
    const auto result = manager.RescanModules();
    std::string response = "Found " + std::to_string(result.NumberOfDetectedModules) + " module(s)";
    httpd_resp_sendstr(req, response.c_str());
    return ESP_OK;
}

esp_err_t configuration_create_filter_handler(httpd_req_t *req)
{
    char body[256];
    int received = httpd_req_recv(req, body, sizeof(body));
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive body");
        return ESP_FAIL;
    }

    body[received] = '\0'; // Terminate

    IniReader reader(body);

    std::string filterType;
    std::string filterName;
    size_t numberOfFiltersCreated = 0;

    reader.Process(
        [&filterType, &filterName](const std::string& section) {
            filterType.clear();
            filterName.clear();
        },
        [&filterType, &filterName, &numberOfFiltersCreated](const std::string& section, const std::string& key, const std::string& value) {
            if (section != "Filter") return;
            if (key == "Type") filterType = value;
            if (key == "Name") filterName = value;

            if (!filterType.empty() && !filterName.empty()) {
                manager.CreateFilter(filterType, filterName);
                numberOfFiltersCreated++;
                filterType.clear();
                filterName.clear();
            }
        }
    );

    std::string response = "Created " + std::to_string(numberOfFiltersCreated) + " filter(s)";
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
    nvs_flash_init();       // Required for Wi-Fi
    wifi_init_sta();        // Connect to Wi-Fi

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    manager.Start();

    xTaskCreate(
        ProcessTask,    // Task function
        "Process task", // Name (for debugging)
        4096,           // Stack size in bytes
        NULL,           // Task parameter
        5,              // Task priority (higher = more important)
        NULL            // Optional handle
    );

    start_webserver();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 second
    }
}
