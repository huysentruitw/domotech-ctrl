#pragma once

#ifndef NATIVE_BUILD

#include "esp_event.h"

#include <string_view>

#define WIFI_CONNECTED_BIT BIT0

enum class WifiMode : uint8_t
{
    Off,
    Station,
    AccessPoint,
};

class Wifi final
{
public:
    Wifi() noexcept;

    void Init() noexcept;
    bool SwitchToStationMode(std::string_view ssid, std::string_view password, TickType_t timeoutTicks) noexcept;
    bool SwitchToAccessPointMode(std::string_view ssid, std::string_view password) noexcept;
    void SwitchOff() noexcept;
    int8_t GetRssi() const noexcept;

private:
    const EventGroupHandle_t m_eventGroup;
    WifiMode m_mode;

    static void EventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) noexcept;
    void OnEvent(esp_event_base_t eventBase, int32_t eventId) noexcept;
};

#endif