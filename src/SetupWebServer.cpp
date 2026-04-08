#include "SetupWebServer.h"

#include "KeyVault.h"

#include "esp_http_server.h"
#include "esp_log.h"

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

void SetupWebServer::Start() noexcept
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
