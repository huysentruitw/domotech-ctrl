#pragma once

#ifndef NATIVE_BUILD

#include <Bridge.h>
#include <Filter.h>
#include <Lock.h>
#include <Pin.h>
#include <StringHash.h>

#include "freertos/FreeRTOS.h"
#include "mqtt_client.h"

#include <memory>
#include <string>
#include <unordered_map>

class HomeAssistantBridge final : public Bridge
{
public:
    HomeAssistantBridge();
    ~HomeAssistantBridge();

    void Init(const char* uri, const char* username, const char* password);
    void RegisterFilter(std::weak_ptr<Filter> filter) override;

private:
    const Lock m_syncRoot;
    QueueHandle_t m_queue;
    esp_mqtt_client_handle_t m_client = nullptr;
    std::unordered_map<std::string, std::weak_ptr<Filter>, StringHash, std::equal_to<>> m_filters;

    static void TaskTrampoline(void* arg);
    void TaskLoop();

    void PublishFilter(std::string_view id);
    void PublishFilter(std::string_view id, const Filter& filter) const;

    void PublishState(std::string_view id, DigitalValue state) const;

    static void EventHandler(void* args, esp_event_base_t base, int32_t eventId, void* data);
    void HandleEvent(esp_mqtt_event_handle_t event);

    static std::string SanitizeId(std::string_view id, size_t maxLength = 32) noexcept;
};

#endif
