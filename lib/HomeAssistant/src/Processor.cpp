#include "Processor.h"

#include "Client.h"
#include "EventLoop.h"
#include "IdSanitizer.h"

#include <Filters/SwitchFilter.h>
#include <Filters/LightFilter.h>
#include <LockGuard.h>

#include "esp_log.h"

#define TAG "HA_PROC"

Processor::Processor(Client& client, EventLoop& eventLoop)
    : m_syncRoot()
    , m_client(client)
    , m_eventLoop(eventLoop)
{
}

void Processor::RegisterFilter(std::weak_ptr<Filter> filter) noexcept
{
    const auto filterPtr = filter.lock();
    if (!filterPtr)
        return;

    ESP_LOGI(TAG, "RegisterFilter");
    std::string id = IdSanitizer::Sanitize(filterPtr->GetId());

    {
        LockGuard guard(m_syncRoot);
        m_filters.emplace(id, filter);
    }

    BridgeEvent event{};
    event.Type = BridgeEvent::Type::CompleteFilterRegistration;
    event.IdLength = std::min(id.length(), (size_t)sizeof(event.Id));
    memcpy(event.Id, id.c_str(), event.IdLength);
    m_eventLoop.EnqueueEvent(event);
}

void Processor::Process(const BridgeEvent& event) noexcept
{
    switch (event.Type) {
        case BridgeEvent::Type::MqttConnected:
            OnMqttConnected();
            break;
        case BridgeEvent::Type::MqttData:
            OnMqttData(event);
            break;
        case BridgeEvent::Type::CompleteFilterRegistration:
            OnCompleteFilterRegistration(event);
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

    m_client.Subscribe("domo/flt/+");

    LockGuard guard(m_syncRoot);
    for (auto it = m_filters.begin(); it != m_filters.end();) {
        if (auto filter = it->second.lock()) {
            PublishDeviceDiscovery(filter);
            ++it;
        } else {
            std::string id = it->first;
            it = m_filters.erase(it);
            PublishFilterRemoval(id);
        }
    }
}

void Processor::OnMqttData(const BridgeEvent& event) noexcept
{
    ESP_LOGI(TAG, "OnMqttData");

    std::string_view topic(event.Topic, event.TopicLength);
    std::string_view payload(event.Payload, event.PayloadLength);
    ESP_LOGI(TAG, "OnMqttData (Topic: %.*s, Payload: %.*s)", (int)topic.length(), topic.data(), (int)payload.length(), payload.data());

    if (topic.rfind("domo/flt/", 0) == 0) {
        std::string_view id = topic.substr(strlen("domo/flt/"));
        const auto state = payload == "ON" ? DigitalValue(true) : DigitalValue(false);

        auto it = m_filters.find(id);
        if (it != m_filters.end()) {
            if (auto filter = it->second.lock()) {
                if (filter->GetType() == FilterType::Switch) {
                    auto* switchFilter = static_cast<SwitchFilter*>(filter.get());
                    switchFilter->SetState(state);
                } else if (filter->GetType() == FilterType::Light) {
                    auto* lightFilter = static_cast<LightFilter*>(filter.get());
                    lightFilter->SetState(state);
                }
            }
        } else {
            // TODO: Unpublish filter!
        }
    }
}

void Processor::OnCompleteFilterRegistration(const BridgeEvent& event) noexcept
{
    ESP_LOGI(TAG, "OnCompleteFilterRegistration");
    std::string_view id(event.Id, event.IdLength);
    if (auto filter = TryGetFilterById(id)) {
        PublishDeviceDiscovery(filter);
        SubscribeToStateChanges(filter);
    }
}

void Processor::OnPublishState(const BridgeEvent& event) noexcept
{
    ESP_LOGI(TAG, "OnPublishState");
    std::string_view id(event.Id, event.IdLength);
    auto state = std::get<DigitalValue>(event.State);

    char topic[64];
    snprintf(topic, sizeof(topic), "domo/flt/%.*s/state", (int)id.size(), id.data());

    const char* payload = (bool)state ? "ON" : "OFF";
    m_client.Publish(topic, payload, false);
}

void Processor::OnShutdown() noexcept
{
    ESP_LOGI(TAG, "OnShutdown");
}

void Processor::PublishDeviceDiscovery(std::shared_ptr<Filter> filter) noexcept
{
    std::string id = IdSanitizer::Sanitize(filter->GetId());
    ESP_LOGI(TAG, "PublishFilterDiscovery (Filter: %s)", id.c_str());

    char topic[64];
    char payload[512];

    if (filter->GetType() == FilterType::Switch) {
        snprintf(topic, sizeof(topic), "homeassistant/switch/%s/config", id.c_str());

        snprintf(payload, sizeof(payload),
            "{"
            "\"unique_id\": \"%s\","
            "\"name\": \"%s\","
            "\"state_topic\": \"domo/flt/%s/state\","
            "\"command_topic\": \"domo/flt/%s\","
            "\"payload_on\": \"ON\","
            "\"payload_off\": \"OFF\","
            "\"optimistic\": false,"
            "\"retain\": true"
            "}",
            id.c_str(),
            id.c_str(),
            id.c_str(),
            id.c_str());

        m_client.Publish(topic, payload, true);
        return;
    }
    
    if (filter->GetType() == FilterType::Light) {
        snprintf(topic, sizeof(topic), "homeassistant/light/%s/config", id.c_str());

        snprintf(payload, sizeof(payload),
            "{"
            "\"unique_id\": \"%s\","
            "\"name\": \"%s\","
            "\"state_topic\": \"domo/flt/%s/state\","
            "\"command_topic\": \"domo/flt/%s\","
            "\"payload_on\": \"ON\","
            "\"payload_off\": \"OFF\","
            "\"optimistic\": false,"
            "\"retain\": true"
            "}",
            id.c_str(),
            id.c_str(),
            id.c_str(),
            id.c_str());

        m_client.Publish(topic, payload, true);
        return;
    }
}

void Processor::SubscribeToStateChanges(std::shared_ptr<Filter> filter) noexcept
{
    std::string id = IdSanitizer::Sanitize(filter->GetId());
    ESP_LOGI(TAG, "SubscribeToFilterStateChanges (Filter: %s)", id.c_str());

    if (filter->GetType() == FilterType::Switch) {
        auto& switchFilter = static_cast<SwitchFilter&>(*filter);
        switchFilter.SetStateCallback([this, id](SwitchFilter& sender, DigitalValue state)
        {
            ESP_LOGI(TAG, "StateCallback");
            BridgeEvent event{};
            event.Type = BridgeEvent::Type::PublishState;
            event.IdLength = std::min(id.length(), (size_t)sizeof(event.Id));
            memcpy(event.Id, id.data(), event.IdLength);
            event.State = state;
            m_eventLoop.EnqueueEvent(event);
        });
        return;
    }

    if (filter->GetType() == FilterType::Light) {
        auto& lightFilter = static_cast<LightFilter&>(*filter);
        lightFilter.SetStateCallback([this, id](LightFilter& sender, DigitalValue state)
        {
            ESP_LOGI(TAG, "StateCallback");
            BridgeEvent event{};
            event.Type = BridgeEvent::Type::PublishState;
            event.IdLength = std::min(id.length(), (size_t)sizeof(event.Id));
            memcpy(event.Id, id.data(), event.IdLength);
            event.State = state;
            m_eventLoop.EnqueueEvent(event);
        });
        return;
    }
}

void Processor::PublishFilterRemoval(std::string_view id) noexcept
{
    // TODO
}

std::shared_ptr<Filter> Processor::TryGetFilterById(std::string_view id) noexcept
{
    LockGuard guard(m_syncRoot);
    auto it = m_filters.find(id);
    return it != m_filters.end()
        ? it->second.lock()
        : nullptr;
}
