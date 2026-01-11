#include "HomeAssistantBridge.h"

#include <PinFactory.h>

#ifndef NATIVE_BUILD

void HomeAssistantBridge::Init(const char* uri, const char* username, const char* password)
{
    esp_mqtt_client_config_t config = {};
    config.broker.address.uri = uri;
    config.credentials.username = username;
    config.credentials.authentication.password = password;

    m_client = esp_mqtt_client_init(&config);
    esp_mqtt_client_register_event(m_client, MQTT_EVENT_ANY, &HomeAssistantBridge::EventHandler, this);
    esp_mqtt_client_start(m_client);
}

void HomeAssistantBridge::RegisterFilter(std::weak_ptr<Filter> filter)
{
    const auto filterPtr = filter.lock();
    if (!filterPtr)
        return;

    if (filterPtr->GetType() == FilterType::Toggle) {
        const auto toggleFilter = static_cast<const ToggleFilter&>(*filterPtr);
        PublishSwitch(toggleFilter);

        const auto statePin = PinFactory::CreateInputPin<DigitalValue>([this, toggleFilter](const Pin& pin)
        {
            std::string id = CreateId(toggleFilter.GetName());
            PublishState(id, pin.GetStateAs<DigitalValue>());
        });
        m_pins.emplace_back(statePin);

        Pin::Connect(statePin, toggleFilter.GetOutputPins()[0]);
    }
}

void HomeAssistantBridge::PublishSwitch(const ToggleFilter& filter) const
{
    if (m_client == nullptr)
        return;

    std::string name = filter.GetName();
    std::string id = CreateId(name);

    char topic[64];
    snprintf(topic, sizeof(topic), "homeassistant/switch/%s/config", id.c_str());

    static char payload[512];

    snprintf(payload, sizeof(payload),
        "{"
        "\"unique_id\": \"%s\","
        "\"name\": \"%s\","
        "\"state_topic\": \"domo/flt/%s\","
        "\"command_topic\": \"domo/flt/%s\","
        "\"payload_on\": \"ON\","
        "\"payload_off\": \"OFF\","
        "\"optimistic\": false,"
        "\"qos\": 0,"
        "\"retain\": true"
        "}", id.c_str(), name.c_str(), id.c_str(), id.c_str());

    esp_mqtt_client_publish(m_client, topic, payload, 0, 1, true);
}

void HomeAssistantBridge::PublishState(std::string_view id, DigitalValue state) const
{
    if (m_client == nullptr)
        return;

    // char id[16];
    // snprintf(id, sizeof(id), "A%u/BTN%u", GetAddress(), buttonIndex);
    // m_statePublisher.PublishState(id, state);

    char topic[64];
    snprintf(topic, sizeof(topic), "domo/flt/%.*s", (int)id.size(), id.data());

    const char* payload = (bool)state ? "ON" : "OFF";
    esp_mqtt_client_publish(m_client, topic, payload, /*len*/0, /*qos*/1, /*retain*/false);
}

void HomeAssistantBridge::EventHandler(void* args, esp_event_base_t base, int32_t eventId, void* data)
{
    auto* self = static_cast<HomeAssistantBridge*>(args);
    self->HandleEvent(static_cast<esp_mqtt_event_handle_t>(data));
}

void HomeAssistantBridge::HandleEvent(esp_mqtt_event_handle_t event)
{
    switch (static_cast<int>(event->event_id)) {
        case MQTT_EVENT_CONNECTED:
            // TODO HA expects devices to re-publish device discovery
            
            esp_mqtt_client_subscribe(event->client, "domo/flt/+", 1);
            break;
        case MQTT_EVENT_DATA:
            std::string_view topic(event->topic, event->topic_len);
            std::string_view payload(event->data, event->data_len);

            if (topic.rfind("domo/flt/", 0) == 0) {
                // std::string_view id = topic.substr(strlen("domo/flt/"));
                // TODO HandleFilterCommand(id, payload);
            }
            break;
    }
}

std::string HomeAssistantBridge::CreateId(std::string_view input)
{
    std::string result;
    result.reserve(input.size());

    for (char c : input) {
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
