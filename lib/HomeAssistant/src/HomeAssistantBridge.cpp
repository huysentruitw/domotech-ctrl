#include "HomeAssistantBridge.h"

#ifndef NATIVE_BUILD

void HomeAssistantBridge::Init(const char* uri, const char* username, const char* password)
{
    esp_mqtt_client_config_t config = {};
    config.broker.address.uri = uri;
    config.credentials.username = username;
    config.credentials.authentication.password = password;

    m_client = esp_mqtt_client_init(&config);
    // esp_mqtt_client_register_event(m_client, MQTT_EVENT_ANY, &HomeAssistantBridge::EventHandler, this);
    esp_mqtt_client_start(m_client);
}

void HomeAssistantBridge::RegisterFilter(std::weak_ptr<Filter> filter)
{
    const auto filterPtr = filter.lock();
    if (!filterPtr)
        return;

    if (auto togglePtr = std::dynamic_pointer_cast<ToggleFilter>(filterPtr))
        PublishSwitch(*togglePtr);
}

void HomeAssistantBridge::PublishSwitch(const ToggleFilter& filter) const
{
    if (m_client == nullptr)
        return;

    std::string_view name = filter.GetName();

    char topic[64];
    snprintf(topic, sizeof(topic), "homeassistant/device/%s/config", name);

    static char payload[2048];

    size_t offset = 0;

    offset += snprintf(payload + offset, sizeof(payload) - offset,
        "{"
          "\"dev\": {"
            "\"ids\": \"%s\""
          "},"
          "\"o\": {"
            "\"name\": \"domo\""
          "},"
          "\"cmps\": {", name);

    for (int i = 0; i < 8; ++i)
    {
        if (i > 0) offset += snprintf(payload + offset, sizeof(payload) - offset, ",");

        offset += snprintf(payload + offset, sizeof(payload) - offset,
            "\"%s\": {"
              "\"p\": \"switch\","
              "\"name\": \"BTN%d\","
              "\"state_topic\": \"domo/A3/BTN%d\","
              "\"payload_on\": \"ON\","
              "\"payload_off\": \"OFF\","
              "\"unique_id\": \"A3_BTN%d\""
            "}", name, i, i, i);
    }

    offset += snprintf(payload + offset, sizeof(payload) - offset,
          "}"
        "}");

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
    sniprintf(topic, sizeof(topic), "domo/%.*s", (int)id.size(), id.data());

    const char* payload = (bool)state ? "ON" : "OFF";
    esp_mqtt_client_publish(m_client, topic, payload, /*len*/0, /*qos*/1, /*retain*/false);
}

// void HomeAssistantBridge::EventHandler(void* args, esp_event_base_t base, int32_t eventId, void* data)
// {
//     auto* self = static_cast<HomeAssistantBridge*>(args);
//     self->HandleEvent(static_cast<esp_mqtt_event_handle_t>(data));
// }

// void HomeAssistantBridge::HandleEvent(esp_mqtt_event_handle_t event)
// {
//     switch (event->event_id) {
//         case MQTT_EVENT_CONNECTED:
//             // HA expects devices to re-publish device discovery
//             // Need to re-subscribe to state topics
//             break;
//         case MQTT_EVENT_DATA:
//             break;
//     }
// }

#endif
