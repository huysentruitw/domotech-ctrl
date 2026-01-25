#ifndef NATIVE_BUILD

#include "Wifi.h"

#include "esp_log.h"
#include "esp_wifi.h"

Wifi::Wifi() noexcept
    : m_eventGroup(xEventGroupCreate())
    , m_mode(WifiMode::Off)
{
}

void Wifi::Init() noexcept
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi::EventHandler, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &Wifi::EventHandler, this));

    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
}

bool Wifi::SwitchToStationMode(std::string_view ssid, std::string_view password, TickType_t timeoutTicks) noexcept
{
    if (m_mode == WifiMode::Station)
        return true;

    SwitchOff();

    xEventGroupClearBits(m_eventGroup, WIFI_CONNECTED_BIT);

    wifi_config_t config = {};
    snprintf((char*)config.sta.ssid, sizeof(config.sta.ssid), "%.*s", (int)ssid.length(), ssid.data());
    snprintf((char*)config.sta.password, sizeof(config.sta.password), "%.*s", (int)password.length(), password.data());

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
    ESP_ERROR_CHECK(esp_wifi_start());

    m_mode = WifiMode::Station;

    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(78));

    EventBits_t bits = xEventGroupWaitBits(m_eventGroup, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, timeoutTicks);

    if ((bits & WIFI_CONNECTED_BIT) == 0)
    {
        ESP_LOGW("WIFI", "WiFi STA timeout while connecting to %s (%s)", config.sta.ssid, config.sta.password);
        return false;
    }

    ESP_LOGI("WIFI", "WiFi STA connected to %s", config.sta.ssid);
    return true;            
}

bool Wifi::SwitchToAccessPointMode(std::string_view ssid, std::string_view password) noexcept
{
    if (m_mode == WifiMode::AccessPoint)
        return true;

    SwitchOff();

    wifi_config_t config = {};
    snprintf((char*)config.ap.ssid, sizeof(config.ap.ssid), "%.*s", (int)ssid.length(), ssid.data());
    snprintf((char*)config.ap.password, sizeof(config.ap.password), "%.*s", (int)password.length(), password.data());
    config.ap.ssid_len = (uint8_t)ssid.length();
    config.ap.channel = 6;
    config.ap.max_connection = 4;
    config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &config));
    ESP_ERROR_CHECK(esp_wifi_start());

    m_mode = WifiMode::AccessPoint;

    ESP_LOGI("WIFI", "AP started, SSID: %s, pw: %s", config.ap.ssid, config.ap.password);
    return true;
}

void Wifi::SwitchOff() noexcept
{
    if (m_mode == WifiMode::Off)
        return;
    
    if (m_mode == WifiMode::Station)
        ESP_ERROR_CHECK(esp_wifi_disconnect());

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    m_mode = WifiMode::Off;
}

int8_t Wifi::GetRssi() const noexcept
{
    if (m_mode == WifiMode::Station)
    {
        wifi_ap_record_t apInfo;
        if (esp_wifi_sta_get_ap_info(&apInfo) == ESP_OK)
            return apInfo.rssi;
    }

    return INT8_MIN;
}

void Wifi::EventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) noexcept
{
    auto wifi = static_cast<Wifi*>(arg);
    wifi->OnEvent(event_base, event_id);
}

void Wifi::OnEvent(esp_event_base_t eventBase, int32_t eventId) noexcept
{
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START)
    {
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
    else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(m_eventGroup, WIFI_CONNECTED_BIT);
    }
}

#endif
