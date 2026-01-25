#pragma once

#include "nvs.h"
#include "nvs_flash.h"

#include <string>

#define NVS_SECRETS_NAMESPACE   "secrets"
#define NVS_VERSION             "ver"
#define NVS_WIFI_SSID_KEY       "ssid"
#define NVS_WIFI_PASS_KEY       "pass"
#define NVS_HA_MQTT_URI_KEY     "ha_uri"
#define NVS_HA_MQTT_USER_KEY    "ha_user"
#define NVS_HA_MQTT_PASS_KEY    "ha_pass"

#define CURRENT_VERSION         1

struct Secrets
{
    char WifiSsid[32];
    char WifiPass[64];
    char HaMqttUri[64];
    char HaMqttUser[32];
    char HaMqttPass[32];
};

class KeyVault
{
public:
    KeyVault() = delete; // Prevent instantiation of static class

    static bool LoadSecrets(Secrets& secrets) noexcept
    {
        nvs_handle_t handle;
        if (nvs_open(NVS_SECRETS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
            return false;

        size_t wifiSsidLen = sizeof(secrets.WifiSsid);
        size_t wifiPassLen = sizeof(secrets.WifiPass);
        size_t haMqttUriLen = sizeof(secrets.HaMqttUri);
        size_t haMqttUserLen = sizeof(secrets.HaMqttUser);
        size_t haMqttPassLen = sizeof(secrets.HaMqttPass);

        uint8_t version;
        bool success = (nvs_get_u8(handle, NVS_VERSION, &version) == ESP_OK) && version == CURRENT_VERSION
                    && (nvs_get_str(handle, NVS_WIFI_SSID_KEY, secrets.WifiSsid, &wifiSsidLen) == ESP_OK)
                    && (nvs_get_str(handle, NVS_WIFI_PASS_KEY, secrets.WifiPass, &wifiPassLen) == ESP_OK)
                    && (nvs_get_str(handle, NVS_HA_MQTT_URI_KEY, secrets.HaMqttUri, &haMqttUriLen) == ESP_OK)
                    && (nvs_get_str(handle, NVS_HA_MQTT_USER_KEY, secrets.HaMqttUser, &haMqttUserLen) == ESP_OK)
                    && (nvs_get_str(handle, NVS_HA_MQTT_PASS_KEY, secrets.HaMqttPass, &haMqttPassLen) == ESP_OK);

        nvs_close(handle);
        return success;
    }

    static bool SaveSecrets(const Secrets& secrets) noexcept
    {
        nvs_handle_t handle;
        if (nvs_open(NVS_SECRETS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK)
            return false;

        bool success = (nvs_set_u8(handle, NVS_VERSION, CURRENT_VERSION) == ESP_OK)
                    && (nvs_set_str(handle, NVS_WIFI_SSID_KEY, secrets.WifiSsid) == ESP_OK)
                    && (nvs_set_str(handle, NVS_WIFI_PASS_KEY, secrets.WifiPass) == ESP_OK)
                    && (nvs_set_str(handle, NVS_HA_MQTT_URI_KEY, secrets.HaMqttUri) == ESP_OK)
                    && (nvs_set_str(handle, NVS_HA_MQTT_USER_KEY, secrets.HaMqttUser) == ESP_OK)
                    && (nvs_set_str(handle, NVS_HA_MQTT_PASS_KEY, secrets.HaMqttPass) == ESP_OK);

        if (success)
            success = (nvs_commit(handle) == ESP_OK);

        nvs_close(handle);
        return success;
    }

    static bool Clear() noexcept
    {
        nvs_handle_t handle;
        if (nvs_open(NVS_SECRETS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK)
            return false;

        nvs_erase_all(handle);
        bool success = nvs_commit(handle) == ESP_OK;
        nvs_close(handle);
        return success;
    }
};
