#include "DimmableLightDevice.h"
#include "IdSanitizer.h"

#include <PinFactory.h>

DimmableLightDevice::DimmableLightDevice(const std::shared_ptr<DimmerFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept
    : Device(filter, eventBus)
{
}

size_t DimmableLightDevice::BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength, "homeassistant/light/%.*s/config", (int)id.length(), id.data());
}

size_t DimmableLightDevice::BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength,
        "{"
        "\"unique_id\": \"%.*s\","
        "\"name\": \"%.*s\","
        "\"state_topic\": \"domo/dev/%.*s/state\","
        "\"command_topic\": \"domo/dev/%.*s/switch\","
        "\"brightness_scale\": 100,"
        "\"brightness_state_topic\": \"domo/dev/%.*s/brightness\","
        "\"brightness_command_topic\": \"domo/dev/%.*s/brightness/set\","
        "\"payload_on\": \"ON\","
        "\"payload_off\": \"OFF\","
        "\"optimistic\": false,"
        "\"retain\": true"
        "}",
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data());
}

void DimmableLightDevice::SubscribeToStateChanges() noexcept
{
    if (m_tap)
        return; // Already subscribed

    auto filter = TryGetFilter();
    if (!filter)
        return;

    auto controlPin = filter->GetControlOutputPin();
    m_tap = PinFactory::CreateInputPin<DimmerControlValue>(this);
    Pin::Connect(m_tap, controlPin);
}

void DimmableLightDevice::ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept
{
    if (subtopic == "switch")
    {
        if (auto filter = TryGetFilter())
            filter->SetState(command == "OFF" ? DimmerControlValue(0) : DimmerControlValue(filter->GetLastOnPercentage(), 2));
    }
    else if (subtopic == "brightness/set")
    {
        if (auto filter = TryGetFilter())
            filter->SetState(DimmerControlValue(ParsePercentage(command), 1));
    }
}

void DimmableLightDevice::EnqueueCurrentState() noexcept
{
    if (!m_tap)
        return;

    auto eventBus = TryGetEventBus();
    if (!eventBus)
        return;

    std::string_view id = GetId();
    const auto state = m_tap->GetStateAs<DimmerControlValue>();

    // Emit status
    uint8_t percentage = state.GetPercentage();
    {
        BridgeEvent event{};
        event.Type = BridgeEvent::Type::PublishState;
        event.TopicLength = snprintf(event.Topic, sizeof(event.Topic), "domo/dev/%.*s/state", (int)id.size(), id.data());
        event.PayloadLength = snprintf(event.Payload, sizeof(event.Payload), percentage > 0 ? "ON" : "OFF");
        event.Retain = true;
        eventBus->EnqueueEvent(event);
    }

    // Emit brightness
    if (percentage > 0)
    {
        BridgeEvent event{};
        event.Type = BridgeEvent::Type::PublishState;
        event.TopicLength = snprintf(event.Topic, sizeof(event.Topic), "domo/dev/%.*s/brightness", (int)id.size(), id.data());
        event.PayloadLength = snprintf(event.Payload, sizeof(event.Payload), "%d", percentage);
        event.Retain = true;
        eventBus->EnqueueEvent(event);
    }
}

void DimmableLightDevice::OnPinStateChanged(const Pin& pin) noexcept
{
    EnqueueCurrentState();
}

uint8_t DimmableLightDevice::ParsePercentage(std::string_view value) noexcept
{
    uint32_t result = 0;

    for (char c : value)
    {
        if (c < '0' || c > '9')
            return 0;
        
        result = result * 10 + (c - '0');

        if (result > 100)
            return 100;
    }

    return result;
}
