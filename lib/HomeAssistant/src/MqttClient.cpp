#ifndef NATIVE_BUILD

#include "MqttClient.h"

#include "esp_log.h"

#define TAG "HA_CLIENT"

static const char* ToString(esp_mqtt_event_id_t id) noexcept; // forward declaration

MqttClient::MqttClient(const char* uri, const char* username, const char* password, IEventBus& eventBus) noexcept
    : m_eventBus(eventBus)
{
    esp_mqtt_client_config_t config = {};
    config.broker.address.uri = uri;
    config.credentials.username = username;
    config.credentials.authentication.password = password;

    m_client = esp_mqtt_client_init(&config);
    esp_mqtt_client_register_event(m_client, MQTT_EVENT_ANY, &MqttClient::EventHandler, this);
}

MqttClient::~MqttClient() noexcept
{
    if (m_client)
    {
        esp_mqtt_client_stop(m_client);
        esp_mqtt_client_destroy(m_client);
        m_client = nullptr;
    }
}

void MqttClient::Connect() const noexcept
{
    if (esp_mqtt_client_start(m_client) == ESP_OK)
    {
        ESP_LOGI(TAG, "Connected");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to connect");
    }
}

void MqttClient::Subscribe(const char* topic) const noexcept
{
    ESP_LOGI(TAG, "Subscribe to %s", topic);
    esp_mqtt_client_subscribe(m_client, topic, 1);
}

void MqttClient::Publish(const char* topic, const char* payload, bool retain) const noexcept
{
    ESP_LOGI(TAG, "Publish to %s (Retain: %s)", topic, retain ? "true" : "false");
    esp_mqtt_client_publish(m_client, topic, payload, 0, 1, retain);
}

void MqttClient::EventHandler(void* args, esp_event_base_t base, int32_t eventId, void* data) noexcept
{
    static_cast<MqttClient*>(args)->ForwardEvent(static_cast<esp_mqtt_event_handle_t>(data));
}

void MqttClient::ForwardEvent(esp_mqtt_event_handle_t handle) noexcept
{
    ESP_LOGI(TAG, "Event %s", ToString(handle->event_id));

    BridgeEvent event{};

    switch (static_cast<int>(handle->event_id))
    {
        case MQTT_EVENT_CONNECTED:
            event.Type = BridgeEvent::Type::MqttConnected;
            m_eventBus.EnqueueEvent(event);
            break;

        case MQTT_EVENT_DATA:
            event.Type = BridgeEvent::Type::MqttData;
            event.TopicLength = std::min((size_t)handle->topic_len, (size_t)sizeof(event.Topic));
            memcpy(event.Topic, handle->topic, event.TopicLength);
            event.PayloadLength = std::min((size_t)handle->data_len, (size_t)sizeof(event.Payload));
            memcpy(event.Payload, handle->data, event.PayloadLength);
            m_eventBus.EnqueueEvent(event);
            break;
    }
}

static const char* ToString(esp_mqtt_event_id_t id) noexcept
{
    switch (id)
    {
        case MQTT_EVENT_ANY:             return "MQTT_EVENT_ANY";
        case MQTT_EVENT_ERROR:           return "MQTT_EVENT_ERROR";
        case MQTT_EVENT_CONNECTED:       return "MQTT_EVENT_CONNECTED";
        case MQTT_EVENT_DISCONNECTED:    return "MQTT_EVENT_DISCONNECTED";
        case MQTT_EVENT_SUBSCRIBED:      return "MQTT_EVENT_SUBSCRIBED";
        case MQTT_EVENT_UNSUBSCRIBED:    return "MQTT_EVENT_UNSUBSCRIBED";
        case MQTT_EVENT_PUBLISHED:       return "MQTT_EVENT_PUBLISHED";
        case MQTT_EVENT_DATA:            return "MQTT_EVENT_DATA";
        case MQTT_EVENT_BEFORE_CONNECT:  return "MQTT_EVENT_BEFORE_CONNECT";
        case MQTT_EVENT_DELETED:         return "MQTT_EVENT_DELETED";
        case MQTT_USER_EVENT:            return "MQTT_USER_EVENT";
        default:                         return "UNKNOWN_MQTT_EVENT";
    }
}

#endif /* NATIVE_BUILD */
