#pragma once

#ifndef NATIVE_BUILD

#include <Bridge.h>

#include <Filters/ToggleFilter.h>
#include <Pin.h>

#include "mqtt_client.h"

#include <memory>
#include <string>

class HomeAssistantBridge final : public Bridge
{
public:
    void Init(const char* uri, const char* username, const char* password);
    void RegisterFilter(std::weak_ptr<Filter> filter) override;

private:
    esp_mqtt_client_handle_t m_client = nullptr;
    std::vector<std::shared_ptr<Pin>> m_pins;

    void PublishSwitch(const ToggleFilter& filter) const;
    void PublishState(std::string_view id, DigitalValue state) const;

    static void EventHandler(void* args, esp_event_base_t base, int32_t eventId, void* data);
    void HandleEvent(esp_mqtt_event_handle_t event);

    static std::string CreateId(std::string_view input);
};

#endif
