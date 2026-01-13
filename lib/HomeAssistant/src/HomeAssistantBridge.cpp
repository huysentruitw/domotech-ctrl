#include "HomeAssistantBridge.h"

#ifndef NATIVE_BUILD

#include <Filters/SwitchFilter.h>
#include <Filters/LightFilter.h>
#include <PinFactory.h>

#include <algorithm>

#include "esp_log.h"

enum class BridgeEventType
{
    PublishFilter,
    MqttConnected,
    MqttData,
    PublishState,
    Shutdown,
};

struct BridgeEvent
{
    BridgeEventType Type;

    char FilterId[32];            // For PublishFilter & PublishState
    size_t FilterIdLength;        // For PublishFilter & PublishState
    PinState State;               // For PublishState

    char Topic[64];               // For MqttData
    size_t TopicLength;           // For MqttData
    char Payload[64];             // For MqttData
    size_t PayloadLength;         // For MqttData
};

HomeAssistantBridge::HomeAssistantBridge()
    : m_syncRoot()
{
    ESP_LOGI("HA", "Construct");
    m_queue = xQueueCreate(10, sizeof(BridgeEvent));
}

HomeAssistantBridge::~HomeAssistantBridge()
{
    ESP_LOGI("HA", "Destruct");
    BridgeEvent ev{};
    ev.Type = BridgeEventType::Shutdown;
    xQueueSend(m_queue, &ev, 0);

    if (m_client) {
        esp_mqtt_client_stop(m_client);
    }
}

void HomeAssistantBridge::Init(const char* uri, const char* username, const char* password)
{
    ESP_LOGI("HA", "Init");
    esp_mqtt_client_config_t config = {};
    config.broker.address.uri = uri;
    config.credentials.username = username;
    config.credentials.authentication.password = password;

    m_client = esp_mqtt_client_init(&config);
    esp_mqtt_client_register_event(m_client, MQTT_EVENT_ANY, &HomeAssistantBridge::EventHandler, this);
    esp_mqtt_client_start(m_client);

    xTaskCreate(&HomeAssistantBridge::TaskTrampoline, "HA_BRIDGE", 4096, this, 5, NULL);
}

void HomeAssistantBridge::TaskTrampoline(void* arg)
{
    static_cast<HomeAssistantBridge*>(arg)->TaskLoop();
}

void HomeAssistantBridge::TaskLoop()
{
    ESP_LOGI("HA", "Task loop waiting for events...");
    while (true) {
        BridgeEvent ev;
        xQueueReceive(m_queue, &ev, portMAX_DELAY);

        if (ev.Type == BridgeEventType::Shutdown) {
            ESP_LOGI("HA", "Handle Shutdown");
            break;
        }

        if (ev.Type == BridgeEventType::PublishFilter) {
            ESP_LOGI("HA", "Handle PublishFilter");
            std::string_view id(ev.FilterId, ev.FilterIdLength);
            PublishFilter(id);
            ESP_LOGI("HA", "Exit: Handle PublishFilter");
        }
        
        else if (ev.Type == BridgeEventType::MqttConnected) {
            ESP_LOGI("HA", "Handle MqttConnected");
            esp_mqtt_client_subscribe(m_client, "domo/flt/+", 1);

            for (auto it = m_filters.begin(); it != m_filters.end();) {
                if (auto filter = it->second.lock()) {
                    PublishFilter(it->first, *filter);
                    ++it;
                } else {
                    it = m_filters.erase(it);
                }
            }

            ESP_LOGI("HA", "Exit: Handle MqttConnected");
        }

        else if (ev.Type == BridgeEventType::MqttData) {
            std::string_view topic(ev.Topic, ev.TopicLength);
            std::string_view payload(ev.Payload, ev.PayloadLength);
            ESP_LOGI("HA", "Handle MqttData (topic: %.*s, payload: %.*s)", (int)topic.length(), topic.data(), (int)payload.length(), payload.data());

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
                }
            }

            ESP_LOGI("HA", "Exit: Handle MqttData");
        }

        else if (ev.Type == BridgeEventType::PublishState) {
            ESP_LOGI("HA", "Handle PublishState");
            std::string_view id(ev.FilterId, ev.FilterIdLength);
            PublishState(id, std::get<DigitalValue>(ev.State));
            ESP_LOGI("HA", "Exit: Handle PublishState");
        }
    }

    ESP_LOGI("HA", "Exit: TaskLoop");

    vTaskDelete(NULL);
}

void HomeAssistantBridge::RegisterFilter(std::weak_ptr<Filter> filter)
{
    const auto filterPtr = filter.lock();
    if (!filterPtr)
        return;

    ESP_LOGI("HA", "RegisterFilter");
    std::string id = SanitizeId(filterPtr->GetId());
    m_filters.emplace(id, filter);

    BridgeEvent ev{};
    ev.Type = BridgeEventType::PublishFilter;
    ev.FilterIdLength = std::min(id.length(), (size_t)sizeof(ev.FilterId));
    memcpy(ev.FilterId, id.c_str(), ev.FilterIdLength);
    xQueueSend(m_queue, &ev, portMAX_DELAY);
    ESP_LOGI("HA", "Exit: RegisterFilter");
}

void HomeAssistantBridge::PublishFilter(std::string_view id)
{
    auto it = m_filters.find(id);
    if (it == m_filters.end())
        return;

    const auto filter = it->second.lock();
    if (filter == nullptr)
        return;

    std::string filterId = it->first;

    if (filter->GetType() == FilterType::Switch) {
        auto& switchFilter = static_cast<SwitchFilter&>(*filter);
        PublishFilter(id, switchFilter);

        switchFilter.SetStateCallback([this, filterId](SwitchFilter& sender, DigitalValue state)
        {
            ESP_LOGI("HA", "StateCallback");
            BridgeEvent ev{};
            ev.Type = BridgeEventType::PublishState;
            ev.FilterIdLength = std::min(filterId.length(), (size_t)sizeof(ev.FilterId));
            memcpy(ev.FilterId, filterId.data(), ev.FilterIdLength);
            ev.State = state;
            xQueueSend(m_queue, &ev, portMAX_DELAY);
            ESP_LOGI("HA", "Exit: StateCallback");
        });

        
    } else if (filter->GetType() == FilterType::Light) {
        auto& lightFilter = static_cast<LightFilter&>(*filter);
        PublishFilter(id, lightFilter);

        lightFilter.SetStateCallback([this, filterId](LightFilter& sender, DigitalValue state)
        {
            ESP_LOGI("HA", "StateCallback");
            BridgeEvent ev{};
            ev.Type = BridgeEventType::PublishState;
            ev.FilterIdLength = std::min(filterId.length(), (size_t)sizeof(ev.FilterId));
            memcpy(ev.FilterId, filterId.data(), ev.FilterIdLength);
            ev.State = state;
            xQueueSend(m_queue, &ev, portMAX_DELAY);
            ESP_LOGI("HA", "Exit: StateCallback");
        });
    }
}

void HomeAssistantBridge::PublishFilter(std::string_view id, const Filter& filter) const
{
    if (m_client == nullptr)
        return;

    ESP_LOGI("HA", "PublishFilter");

    // Use original id (unsanitized) as name
    std::string name = filter.GetId();

    char topic[64];
    char payload[512];

    if (filter.GetType() == FilterType::Switch) {
        snprintf(topic, sizeof(topic), "homeassistant/switch/%.*s/config", (int)id.size(), id.data());

        snprintf(payload, sizeof(payload),
            "{"
            "\"unique_id\": \"%.*s\","
            "\"name\": \"%s\","
            "\"state_topic\": \"domo/flt/%.*s/state\","
            "\"command_topic\": \"domo/flt/%.*s\","
            "\"payload_on\": \"ON\","
            "\"payload_off\": \"OFF\","
            "\"optimistic\": false,"
            "\"retain\": true"
            "}",
            (int)id.size(), id.data(),
            name.c_str(),
            (int)id.size(), id.data(),
            (int)id.size(), id.data());

        esp_mqtt_client_publish(m_client, topic, payload, 0, 1, true);
    } else if (filter.GetType() == FilterType::Light) {
        snprintf(topic, sizeof(topic), "homeassistant/light/%.*s/config", (int)id.size(), id.data());

        snprintf(payload, sizeof(payload),
            "{"
            "\"unique_id\": \"%.*s\","
            "\"name\": \"%s\","
            "\"state_topic\": \"domo/flt/%.*s/state\","
            "\"command_topic\": \"domo/flt/%.*s\","
            "\"payload_on\": \"ON\","
            "\"payload_off\": \"OFF\","
            "\"optimistic\": false,"
            "\"retain\": true"
            "}",
            (int)id.size(), id.data(),
            name.c_str(),
            (int)id.size(), id.data(),
            (int)id.size(), id.data());

        esp_mqtt_client_publish(m_client, topic, payload, 0, 1, true);
    }

    ESP_LOGI("HA", "Exit: PublishFilter");
}

void HomeAssistantBridge::PublishState(std::string_view id, DigitalValue state) const
{
    if (m_client == nullptr)
        return;

    ESP_LOGI("HA", "PublishState");

    char topic[64];
    snprintf(topic, sizeof(topic), "domo/flt/%.*s/state", (int)id.size(), id.data());

    const char* payload = (bool)state ? "ON" : "OFF";
    esp_mqtt_client_publish(m_client, topic, payload, /*len*/0, /*qos*/1, /*retain*/false);

    ESP_LOGI("HA", "Exit: PublishState");
}

void HomeAssistantBridge::EventHandler(void* args, esp_event_base_t base, int32_t eventId, void* data)
{
    auto* self = static_cast<HomeAssistantBridge*>(args);
    self->HandleEvent(static_cast<esp_mqtt_event_handle_t>(data));
}

void HomeAssistantBridge::HandleEvent(esp_mqtt_event_handle_t event)
{
    ESP_LOGI("HA", "HandleEvent (%d)", event->event_id);
    BridgeEvent ev{};
    switch (static_cast<int>(event->event_id)) {
        case MQTT_EVENT_CONNECTED:
            ev.Type = BridgeEventType::MqttConnected;
            xQueueSend(m_queue, &ev, portMAX_DELAY);
            break;
        case MQTT_EVENT_DATA:
            ev.Type = BridgeEventType::MqttData;
            ev.TopicLength = std::min((size_t)event->topic_len, (size_t)sizeof(ev.Topic));
            memcpy(ev.Topic, event->topic, ev.TopicLength);
            ev.PayloadLength = std::min((size_t)event->data_len, (size_t)sizeof(ev.Payload));
            memcpy(ev.Payload, event->data, ev.PayloadLength);
            xQueueSend(m_queue, &ev, portMAX_DELAY);
            break;
    }
    ESP_LOGI("HA", "Exit: HandleEvent");
}

std::string HomeAssistantBridge::SanitizeId(std::string_view id, size_t maxLength) noexcept
{
    std::string result;
    result.reserve(std::min((size_t)id.size(), maxLength));

    for (char c : id) {
        if (result.size() == maxLength)
            break;

        if (c == ' ')
        {
            result.push_back('-');
            continue;
        }

        char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if ((lower >= 'a' && lower <= 'z') || (lower >= '0' && lower <= '9') || lower == '_' || lower == '-') {
            result.push_back(lower);
        }
    }

    return result;
}

#endif
