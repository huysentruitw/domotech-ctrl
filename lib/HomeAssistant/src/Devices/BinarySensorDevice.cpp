#include "BinarySensorDevice.h"
#include "IdSanitizer.h"

#include <PinFactory.h>

BinarySensorDevice::BinarySensorDevice(const std::shared_ptr<DigitalPassthroughFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept
    : Device(filter, eventBus)
{
}

size_t BinarySensorDevice::BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength, "homeassistant/binary_sensor/%.*s/config", (int)id.length(), id.data());
}

size_t BinarySensorDevice::BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength,
        "{"
        "\"unique_id\": \"%.*s\","
        "\"name\": \"%.*s\","
        "\"state_topic\": \"domo/dev/%.*s/state\","
        "\"payload_on\": \"ON\","
        "\"payload_off\": \"OFF\","
        "\"optimistic\": false,"
        "\"retain\": true"
        "}",
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data());
}

void BinarySensorDevice::SubscribeToStateChanges() noexcept
{
    auto filter = TryGetFilter();
    if (!filter)
        return;

    auto outputPin = filter->GetOutputPin();
    m_tap = PinFactory::CreateInputPin<DigitalValue>(this);
    Pin::Connect(m_tap, outputPin);
}

void BinarySensorDevice::ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept
{
    // This device doesn't support commands
}

void BinarySensorDevice::EnqueueCurrentState() noexcept
{
    if (!m_tap)
        return;

    auto eventBus = TryGetEventBus();
    if (!eventBus)
        return;
    
    std::string_view id = GetId();
    BridgeEvent event{};
    event.Type = BridgeEvent::Type::PublishState;
    event.TopicLength = snprintf(event.Topic, sizeof(event.Topic), "domo/dev/%.*s/state", (int)id.length(), id.data());
    event.PayloadLength = snprintf(event.Payload, sizeof(event.Payload), m_tap->GetStateAs<DigitalValue>() ? "ON" : "OFF");
    event.Retain = true;
    eventBus->EnqueueEvent(event);
}

void BinarySensorDevice::OnPinStateChanged(const Pin& pin) noexcept
{
    EnqueueCurrentState();
}
