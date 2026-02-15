#include <format>
#include <string>

#include <FilterFactory.h>
#include <HomeAssistantBridge.h>
#include <IniReader.h>
#include <LittleFsStorage.h>
#include <Manager.h>
#include <Wifi.h>

#include "KeyVault.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "nvs_flash.h"

#define VERSION "1.0"
#define POSIX_TIMEZONE "CET-1CEST,M3.5.0/2,M10.5.0/3" // Belgium

LittleFsStorage storage;
Wifi wifi;
HomeAssistantBridge homeAssistantBridge;
Manager manager(storage, homeAssistantBridge);

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

    int8_t rssi = wifi.GetRssi();
    if (rssi > INT8_MIN)
    {
        response.append("Wifi RSSI: ");
        response.append(std::to_string(rssi));
        response.append("dBm, quality: ");
        const auto quality = rssi <= -100 ? 0 : rssi >= -50 ? 100 : 2 * (rssi + 100);
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
    const auto ini = FilterFactory::GetKnownFiltersIni();
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, ini.c_str());
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
    if (received <= 0)
    {
        httpd_resp_set_hdr(req, "Connection", "close");
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive body");
        return ESP_FAIL;
    }

    body[received] = '\0'; // Terminate

    bool inFilterSection;
    std::string id;
    std::string type;
    std::string connections;
    size_t numberOfFiltersCreated = 0;

    IniReader reader;
    reader.OnSection([&](std::string_view section)
    {
        inFilterSection = (section == "Filter");
        id.clear();
        type.clear();
        connections.clear();
    });
    
    reader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value)
    {
        if (!inFilterSection)
            return;

        if (key == "Id") id = value;
        if (key == "Type") type = value;
        if (key == "Connections") connections = value;

        if (!id.empty() && !type.empty() && !connections.empty())
        {
            if (manager.CreateFilter(id, type, connections).Status == CreateFilterStatus::NoError)
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

esp_err_t reset_handler(httpd_req_t *req)
{
    KeyVault::Clear();
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Reset complete! Please reboot the device.");
    return ESP_OK;
}

esp_err_t setup_handler(httpd_req_t *req)
{
    Secrets secrets = {};
    if (!KeyVault::LoadSecrets(secrets))
        memset(&secrets, 0, sizeof(secrets));

    char html[2045];
    snprintf(html, sizeof(html),
        "<!DOCTYPE html><html><head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "</head><body>"
        "<form method='POST' action='/'>"
        "<h1>Domotech CTRL Setup</h1>"
        "<h2>WiFi configuration</h2>"
        "SSID: <input name='wifi_ssid' value='%s'><br>"
        "Password: <input name='wifi_pass' value='%s' type='password'><br>"
        "<h2>HA MQTT configuration</h2>"
        "MQTT URI: <input name='ha_mqtt_uri' value='%s'><br>"
        "MQTT User: <input name='ha_mqtt_user' value='%s'><br>"
        "MQTT Password: <input name='ha_mqtt_pass' value='%s' type='password'><br>"
        "<input type='submit' value='Save'>"
        "</form>"
        "</body></html>",
        secrets.WifiSsid,
        secrets.WifiPass,
        secrets.HaMqttUri,
        secrets.HaMqttUser,
        secrets.HaMqttPass);

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, html);
    return ESP_OK;
}

void urlDecodeInPlace(char* s)
{
    char* src = s;
    char* dst = s;

    while (*src)
    {
        if (*src == '%' && isxdigit(src[1]) && isxdigit(src[2]))
        {
            char hex[3] = { src[1], src[2], 0 };
            *dst++ = strtol(hex, nullptr, 16);
            src += 3;
        }
        else if (*src == '+')
        {
            *dst++ = ' ';
            src++;
        }
        else
        {
            *dst++ = *src++;
        }
    }

    *dst = '\0';
}

esp_err_t setup_post_handler(httpd_req_t *req)
{
    char buffer[512];
    int total = req->content_len;

    if (total >= sizeof(buffer))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Body too large");
        return ESP_FAIL;
    }

    int received = httpd_req_recv(req, buffer, total);
    if (received <= 0)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to read body");
        return ESP_FAIL;
    }

    buffer[received] = '\0';
    ESP_LOGI("WEB", "Received: %s", buffer);

    auto parseNext = [&](const char* key) -> std::string_view
    {
        char* p = strstr(buffer, key);
        if (!p) return "";
        p += strlen(key);
        char* end = strchr(p, '&');
        return end ? std::string_view(p, end - p) : std::string_view(p);
    };

    std::string_view ssid      = parseNext("wifi_ssid=");
    std::string_view pass      = parseNext("wifi_pass=");
    std::string_view mqtt_uri  = parseNext("ha_mqtt_uri=");
    std::string_view mqtt_user = parseNext("ha_mqtt_user=");
    std::string_view mqtt_pass = parseNext("ha_mqtt_pass=");

    Secrets secrets = {};
    snprintf(secrets.WifiSsid, sizeof(secrets.WifiSsid), "%.*s", (int)ssid.length(), ssid.data());
    snprintf(secrets.WifiPass, sizeof(secrets.WifiPass), "%.*s", (int)pass.length(), pass.data());
    snprintf(secrets.HaMqttUri, sizeof(secrets.HaMqttUri), "%.*s", (int)mqtt_uri.length(), mqtt_uri.data());
    snprintf(secrets.HaMqttUser, sizeof(secrets.HaMqttUser), "%.*s", (int)mqtt_user.length(), mqtt_user.data());
    snprintf(secrets.HaMqttPass, sizeof(secrets.HaMqttPass), "%.*s", (int)mqtt_pass.length(), mqtt_pass.data());
    urlDecodeInPlace(secrets.HaMqttUri);
    KeyVault::SaveSecrets(secrets);

    const char resp[] =
        "<html><body>" "<h1>Saved!</h1>"
        "<p>Your configuration has been stored.</p>"
        "<p>The device will now reboot.</p>"
        "</body></html>";

    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, resp);
    return ESP_OK;
}

void start_setup_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    config.stack_size = 8192;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t setup_uri =
        {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = setup_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &setup_uri);

        httpd_uri_t setup_post_uri =
        {
            .uri       = "/",
            .method    = HTTP_POST,
            .handler   = setup_post_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &setup_post_uri);
    }
}

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    config.stack_size = 8192;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t hello_uri =
        {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &hello_uri);

        httpd_uri_t known_filters_uri =
        {
            .uri       = "/known-filters",
            .method    = HTTP_GET,
            .handler   = known_filters_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &known_filters_uri);

        httpd_uri_t configuration_rescan_uri =
        {
            .uri       = "/configuration/rescan",
            .method    = HTTP_POST,
            .handler   = configuration_rescan_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &configuration_rescan_uri);

        httpd_uri_t configuration_create_filter_uri =
        {
            .uri       = "/configuration/filter",
            .method    = HTTP_POST,
            .handler   = configuration_create_filter_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &configuration_create_filter_uri);

        httpd_uri_t reset_uri =
        {
            .uri       = "/reset",
            .method    = HTTP_POST,
            .handler   = reset_handler,
            .user_ctx  = NULL,
        };
        httpd_register_uri_handler(server, &reset_uri);
    }
}

void ProcessTask(void *arg)
{
    while (true)
    {
        manager.ProcessNext();
        vTaskDelay(1);
    }

    vTaskDelete(NULL);
}

extern "C" void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());  // Required for Wi-Fi
    wifi.Init();

    Secrets secrets = {};
    if (!KeyVault::LoadSecrets(secrets) || !wifi.SwitchToStationMode(secrets.WifiSsid, secrets.WifiPass, pdMS_TO_TICKS(10000)))
    {
        ScanLed led;
        ESP_LOGW("MAIN", "STA failed, starting AP config mode");
        wifi.SwitchToAccessPointMode("DOMOTECH_CTRL", "domotech");
        start_setup_webserver();
        while (true)
        {
            led.Toggle();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    ESP_LOGI("MAIN", "Started!");

    homeAssistantBridge.Init(secrets.HaMqttUri, secrets.HaMqttUser, secrets.HaMqttPass);
    manager.Start();

    xTaskCreate(
        ProcessTask,    // Task function
        "PROCESS",      // Name (for debugging)
        4096,           // Stack size in bytes
        NULL,           // Task parameter
        5,              // Task priority (higher = more important)
        NULL            // Optional handle
    );

    time_init();
    start_webserver();

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
