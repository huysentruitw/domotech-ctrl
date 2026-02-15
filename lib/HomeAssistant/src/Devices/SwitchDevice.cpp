#include "SwitchDevice.h"
#include "IdSanitizer.h"

#include <PinFactory.h>

SwitchDevice::SwitchDevice(const std::shared_ptr<SwitchFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept
    : Device(filter, eventBus)
{
}

bool SwitchDevice::BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    int required = snprintf(buffer, bufferLength, "homeassistant/switch/%.*s/config", (int)id.length(), id.data());
    return required >= 0 && static_cast<size_t>(required) < bufferLength;
}

bool SwitchDevice::BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    int required = snprintf(buffer, bufferLength,
        "{"
        "\"unique_id\": \"%.*s\","
        "\"name\": \"%.*s\","
        "\"state_topic\": \"domo/dev/%.*s/state\","
        "\"command_topic\": \"domo/dev/%.*s/switch\","
        "\"payload_on\": \"ON\","
        "\"payload_off\": \"OFF\","
        "\"optimistic\": false,"
        "\"retain\": true"
        "}",
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data());

    return required >= 0 && static_cast<size_t>(required) < bufferLength;
}

bool SwitchDevice::BuildStateMessages(StateMessageList& list) const noexcept
{
    if (!m_tap)
        return false;

    std::string_view id = GetId();

    StateMessage message{};
    snprintf(message.Topic, sizeof(message.Topic), "domo/dev/%.*s/state", (int)id.length(), id.data());
    snprintf(message.Payload, sizeof(message.Payload), m_tap->GetStateAs<DigitalValue>() ? "ON" : "OFF");
    message.Retain = true;
    return list.Add(message);
}

void SwitchDevice::SubscribeToStateChanges() noexcept
{
    if (m_tap)
        return; // Already subscribed

    auto filter = TryGetFilter();
    if (!filter)
        return;

    auto controlPin = filter->GetControlOutputPin();
    m_tap = PinFactory::CreateInputPin<DigitalValue>(this);
    Pin::Connect(m_tap, controlPin);
}

void SwitchDevice::ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept
{
    if (subtopic != "switch")
        return;

    const auto state = command == "ON" ? DigitalValue(true) : DigitalValue(false);
    if (auto filter = TryGetFilter())
        filter->SetState(state);
}

void SwitchDevice::OnPinStateChanged(const Pin& pin) noexcept
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
