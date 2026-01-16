#include "Processor.h"

#include "Client.h"
#include "EventLoop.h"

#include <LockGuard.h>

#include <cstring>

#ifndef NATIVE_BUILD
    #include "esp_log.h"
#else
    #define ESP_LOGI(...) {}
#endif

#define TAG "HA_PROC"

Processor::Processor(Client& client, EventLoop& eventLoop) noexcept
    : m_syncRoot()
    , m_client(client)
    , m_eventLoop(eventLoop)
{
}

void Processor::RegisterDevice(const std::shared_ptr<IDevice>& device) noexcept
{
    const std::string_view id = device->GetId();
    ESP_LOGI(TAG, "RegisterDevice (Id: %.*s)", (int)id.length(), id.data());

    // Already add the device to the map, so we don't have to pass the pointer over the FreeRTOS queue
    {
        LockGuard guard(m_syncRoot);
        m_devices.emplace(id, device);
    }

    BridgeEvent event{};
    event.Type = BridgeEvent::Type::CompleteDeviceRegistration;
    event.IdLength = std::min(id.length(), (size_t)sizeof(event.Id));
    memcpy(event.Id, id.data(), event.IdLength);
    m_eventLoop.EnqueueEvent(event);
}

void Processor::UnregisterDevice(std::string_view id) noexcept
{
    ESP_LOGI(TAG, "UnregisterDevice (Id: %.*s)", (int)id.length(), id.data());

    if (!TryGetDeviceById(id))
        return;

    BridgeEvent event{};
    event.Type = BridgeEvent::Type::UnregisterDevice;
    event.IdLength = std::min(id.length(), (size_t)sizeof(event.Id));
    memcpy(event.Id, id.data(), event.IdLength);
    m_eventLoop.EnqueueEvent(event);
}

void Processor::Process(const BridgeEvent& event) noexcept
{
    switch (event.Type)
    {
        case BridgeEvent::Type::MqttConnected:
            OnMqttConnected();
            break;
        case BridgeEvent::Type::MqttData:
            OnMqttData(event);
            break;
        case BridgeEvent::Type::CompleteDeviceRegistration:
            OnCompleteDeviceRegistration(event);
            break;
        case BridgeEvent::Type::UnregisterDevice:
            OnUnregisterDevice(event);
            break;
        case BridgeEvent::Type::PublishState:
            OnPublishState(event);
            break;
        case BridgeEvent::Type::Shutdown:
            OnShutdown();
            break;
    }
}

void Processor::OnMqttConnected() noexcept
{
    ESP_LOGI(TAG, "OnMqttConnected");

    m_client.Subscribe("domo/dev/#");

    LockGuard guard(m_syncRoot);
    for (auto& [_, device] : m_devices)
        PublishDeviceDiscovery(*device);
}

void Processor::OnMqttData(const BridgeEvent& event) noexcept
{
    std::string_view topic(event.Topic, event.TopicLength);
    std::string_view payload(event.Payload, event.PayloadLength);
    ESP_LOGI(TAG, "OnMqttData (Topic: %.*s, Payload: %.*s)", (int)topic.length(), topic.data(), (int)payload.length(), payload.data());

    constexpr std::string_view prefix = "domo/dev/";
    if (!topic.starts_with(prefix))
        return;

    std::string_view rest = topic.substr(prefix.length());

    // Find next slash to separate <id> and <subtopic>
    size_t slashPos = rest.find('/');
    if (slashPos == std::string_view::npos)
        return; // No subtopic -> ignore

    std::string_view id = rest.substr(0, slashPos);
    std::string_view subtopic = rest.substr(slashPos + 1);
        
    if (auto device = TryGetDeviceById(id))
        device->ProcessCommand(subtopic, payload);
}

void Processor::OnCompleteDeviceRegistration(const BridgeEvent& event) noexcept
{
    std::string_view id(event.Id, event.IdLength);
    ESP_LOGI(TAG, "OnCompleteDeviceRegistration (Id: %.*s)", (int)id.length(), id.data());
    if (auto device = TryGetDeviceById(id))
    {
        PublishDeviceDiscovery(*device);
        SubscribeToStateChanges(*device);
    }
}

void Processor::OnUnregisterDevice(const BridgeEvent& event) noexcept
{
    std::string_view id(event.Id, event.IdLength);
    ESP_LOGI(TAG, "OnUnregisterDevice (Id: %.*s)", (int)id.length(), id.data());
    if (auto device = TryGetDeviceById(id))
    {
        PublishDeviceRemoval(*device);

        LockGuard guard(m_syncRoot);
        const auto it = m_devices.find(id);
        if (it != m_devices.end())
            m_devices.erase(it);
    }
}

void Processor::OnPublishState(const BridgeEvent& event) noexcept
{
    std::string_view id(event.Id, event.IdLength);
    ESP_LOGI(TAG, "OnPublishState (Id: %.*s)", (int)id.length(), id.data());
    
    if (auto digitalValue = std::get_if<DigitalValue>(&event.State))
    {
        char topic[64];
        snprintf(topic, sizeof(topic), "domo/dev/%.*s/status", (int)id.size(), id.data());
        const char* payload = (bool)*digitalValue ? "ON" : "OFF";
        m_client.Publish(topic, payload, false);
    }
    else if (auto dimmerControlValue = std::get_if<DimmerControlValue>(&event.State))
    {
        uint8_t percentage = dimmerControlValue->GetPercentage();

        char topic[64];
        char payload[8];

        snprintf(topic, sizeof(topic), "domo/dev/%.*s/status", (int)id.size(), id.data());
        const char* statePayload = percentage > 0 ? "ON" : "OFF";
        m_client.Publish(topic, statePayload, false);

        if (percentage > 0)
        {
            snprintf(topic, sizeof(topic), "domo/dev/%.*s/brightness", (int)id.size(), id.data());
            snprintf(payload, sizeof(payload), "%d", percentage);
            m_client.Publish(topic, payload, false);
        }
    }
}

void Processor::OnShutdown() noexcept
{
    ESP_LOGI(TAG, "OnShutdown");
}

void Processor::PublishDeviceDiscovery(const IDevice& device) noexcept
{
    std::string_view id = device.GetId();
    ESP_LOGI(TAG, "PublishDeviceDiscovery (Id: %.*s)", (int)id.length(), id.data());

    char topic[64];
    char payload[512];
    device.BuildDiscoveryTopic(topic, sizeof(topic));
    device.BuildDiscoveryPayload(payload, sizeof(payload));
    m_client.Publish(topic, payload, false);
}

void Processor::PublishDeviceRemoval(const IDevice& device) noexcept
{
    std::string_view id = device.GetId();
    ESP_LOGI(TAG, "PublishDeviceRemoval (Id: %.*s)", (int)id.length(), id.data());

    char topic[64];
    device.BuildDiscoveryTopic(topic, sizeof(topic));
    m_client.Publish(topic, "", true);
}

void Processor::SubscribeToStateChanges(const IDevice& device) noexcept
{
    std::string_view id = device.GetId();
    ESP_LOGI(TAG, "SubscribeToStateChanges (Id: %.*s)", (int)id.length(), id.data());

    device.SetStateCallback(
        [this, id](PinState state)
        {
            BridgeEvent event{};
            event.Type = BridgeEvent::Type::PublishState;
            event.IdLength = std::min(id.length(), (size_t)sizeof(event.Id));
            memcpy(event.Id, id.data(), event.IdLength);
            event.State = state;
            m_eventLoop.EnqueueEvent(event);
        });
}

std::shared_ptr<IDevice> Processor::TryGetDeviceById(std::string_view id) const noexcept
{
    LockGuard guard(m_syncRoot);
    auto it = m_devices.find(id);
    return it != m_devices.end() ? it->second : nullptr;
}
