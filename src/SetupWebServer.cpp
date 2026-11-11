#include "SetupWebServer.h"

#include "KeyVault.h"

#include <FormDecoder.h>

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

esp_err_t setup_post_handler(httpd_req_t *req)
{
    char buffer[512];
    const size_t total = req->content_len;

    if (total >= sizeof(buffer))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Body too large");
        return ESP_FAIL;
    }

    // httpd_req_recv may return fewer bytes than requested, so keep reading until the whole body is in
    size_t received = 0;
    while (received < total)
    {
        const int ret = httpd_req_recv(req, buffer + received, total - received);
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            continue;

        if (ret <= 0)
        {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to read body");
            return ESP_FAIL;
        }

        received += ret;
    }

    const std::string_view body(buffer, received);

    Secrets secrets = {};
    const bool valid =
           FormDecoder::TryGetValue(body, "wifi_ssid", secrets.WifiSsid, sizeof(secrets.WifiSsid))
        && FormDecoder::TryGetValue(body, "wifi_pass", secrets.WifiPass, sizeof(secrets.WifiPass))
        && FormDecoder::TryGetValue(body, "ha_mqtt_uri", secrets.HaMqttUri, sizeof(secrets.HaMqttUri))
        && FormDecoder::TryGetValue(body, "ha_mqtt_user", secrets.HaMqttUser, sizeof(secrets.HaMqttUser))
        && FormDecoder::TryGetValue(body, "ha_mqtt_pass", secrets.HaMqttPass, sizeof(secrets.HaMqttPass));

    if (!valid)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or too long field");
        return ESP_FAIL;
    }

    ESP_LOGI("WEB", "Received configuration for SSID '%s'", secrets.WifiSsid);

    if (!KeyVault::SaveSecrets(secrets))
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to store configuration");
        return ESP_FAIL;
    }

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
