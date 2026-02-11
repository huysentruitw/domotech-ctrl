#pragma once

#include "IEventBus.h"

#ifndef NATIVE_BUILD

#include "mqtt_client.h"

class MqttClient final
{
public:
    MqttClient(const char* uri, const char* username, const char* password, IEventBus& eventBus) noexcept;
    ~MqttClient() noexcept;

    void Connect() const noexcept;

    void Subscribe(const char* topic) const noexcept;
    void Publish(const char* topic, const char* payload, bool retain) const noexcept;

private:
    IEventBus& m_eventBus;
    esp_mqtt_client_handle_t m_client = nullptr;

    static void EventHandler(void* args, esp_event_base_t base, int32_t eventId, void* data) noexcept;
    void ForwardEvent(esp_mqtt_event_handle_t event) noexcept;
};

#else

class MqttClient final
{
public:
    MqttClient(const char* uri, const char* username, const char* password, IEventBus& eventBus) noexcept {};
    ~MqttClient() noexcept {};

    void Connect() const noexcept {};

    void Subscribe(const char* topic) const noexcept {};
    void Publish(const char* topic, const char* payload, bool retain) const noexcept {};
};

#endif
