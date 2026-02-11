#include "Processor.h"

#include <LockGuard.h>

#include <cstring>

#ifndef NATIVE_BUILD
    #include "esp_log.h"
#else
    #define ESP_LOGI(...) {}
#endif

#define TAG "HA_PROC"

Processor::Processor(MqttClient& client, IEventBus& eventBus) noexcept
    : m_syncRoot()
    , m_mqttClient(client)
    , m_eventBus(eventBus)
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
    m_eventBus.EnqueueEvent(event);
}

void Processor::UnregisterDevice(std::string_view id) noexcept
{
    ESP_LOGI(TAG, "UnregisterDevice (Id: %.*s)", (int)id.length(), id.data());

    BridgeEvent event{};
    event.Type = BridgeEvent::Type::UnregisterDevice;
    event.IdLength = std::min(id.length(), (size_t)sizeof(event.Id));
    memcpy(event.Id, id.data(), event.IdLength);
    m_eventBus.EnqueueEvent(event);
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
        case BridgeEvent::Type::PublishNextDiscovery:
            OnPublishNextDiscovery();
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

    m_mqttClient.Subscribe("domo/dev/#");

    {
        LockGuard guard(m_syncRoot);
        while (!m_discoveryQueue.empty())
            m_discoveryQueue.pop();

        if (m_devices.empty())
            return;

        for (auto& [id, _] : m_devices)
            m_discoveryQueue.emplace(id);
    }

    BridgeEvent event{};
    event.Type = BridgeEvent::Type::PublishNextDiscovery;
    m_eventBus.EnqueueEvent(event);
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
        device->SubscribeToStateChanges();
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

void Processor::OnPublishNextDiscovery() noexcept
{
    ESP_LOGI(TAG, "OnPublishNextDiscovery");

    bool hasMore;
    std::string id;

    {
        LockGuard guard(m_syncRoot);
        if (m_discoveryQueue.empty())
            return;

        id = m_discoveryQueue.front();
        m_discoveryQueue.pop();

        hasMore = !m_discoveryQueue.empty();
    }

    if (auto device = TryGetDeviceById(id))
    {
        PublishDeviceDiscovery(*device);

        // device->PublishDiscovery(m_client);
        // device->PublishCurrentState(m_client);
    }

    if (hasMore)
    {
        BridgeEvent event{};
        event.Type = BridgeEvent::Type::PublishNextDiscovery;
        m_eventBus.EnqueueEvent(event);
    }
}

void Processor::OnPublishState(const BridgeEvent& event) noexcept
{
    std::string_view id(event.Id, event.IdLength);
    ESP_LOGI(TAG, "OnPublishState (Id: %.*s)", (int)id.length(), id.data());
    m_mqttClient.Publish(event.Topic, event.Payload, event.Retain);
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
    char payload[640];
    device.BuildDiscoveryTopic(topic, sizeof(topic));
    device.BuildDiscoveryPayload(payload, sizeof(payload));
    m_mqttClient.Publish(topic, payload, false);
}

void Processor::PublishDeviceRemoval(const IDevice& device) noexcept
{
    std::string_view id = device.GetId();
    ESP_LOGI(TAG, "PublishDeviceRemoval (Id: %.*s)", (int)id.length(), id.data());

    char topic[64];
    device.BuildDiscoveryTopic(topic, sizeof(topic));
    m_mqttClient.Publish(topic, "", true);
}

std::shared_ptr<IDevice> Processor::TryGetDeviceById(std::string_view id) const noexcept
{
    LockGuard guard(m_syncRoot);
    auto it = m_devices.find(id);
    return it != m_devices.end() ? it->second : nullptr;
}
