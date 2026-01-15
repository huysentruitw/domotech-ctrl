#pragma once

#include "BridgeEvent.h"

using ClientCallback = void(*)(void* context, const BridgeEvent& event);

#ifndef NATIVE_BUILD

#include "mqtt_client.h"

class Client final
{
public:
    Client(const char* uri, const char* username, const char* password, ClientCallback callback, void* callbackContext) noexcept;
    ~Client() noexcept;

    void Connect() const noexcept;

    void Subscribe(const char* topic) const noexcept;
    void Publish(const char* topic, const char* payload, bool retain) const noexcept;

private:
    ClientCallback m_callback = nullptr;
    void* m_callbackContext = nullptr;
    esp_mqtt_client_handle_t m_client = nullptr;

    static void EventHandler(void* args, esp_event_base_t base, int32_t eventId, void* data) noexcept;
    void ForwardEvent(esp_mqtt_event_handle_t event) noexcept;
};

#else

class Client final
{
public:
    Client(const char* uri, const char* username, const char* password, ClientCallback callback, void* callbackContext) noexcept {};
    ~Client() noexcept {};

    void Connect() const noexcept {};

    void Subscribe(const char* topic) const noexcept {};
    void Publish(const char* topic, const char* payload, bool retain) const noexcept {};
};

#endif
