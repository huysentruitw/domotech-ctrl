#include "CoverDevice.h"
#include "IdSanitizer.h"

#include <PinFactory.h>

CoverDevice::CoverDevice(const std::shared_ptr<ShutterFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept
    : Device(filter, eventBus)
{
}

bool CoverDevice::BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    int required = snprintf(buffer, bufferLength, "homeassistant/cover/%.*s/config", (int)id.length(), id.data());
    return required >= 0 && static_cast<size_t>(required) < bufferLength;
}

bool CoverDevice::BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    int required = snprintf(buffer, bufferLength,
        "{"
        "\"unique_id\": \"%.*s\","
        "\"name\": \"%.*s\","
        "\"state_topic\": \"domo/dev/%.*s/state\","
        "\"command_topic\": \"domo/dev/%.*s/action\","
        "\"payload_open\": \"OPEN\","
        "\"payload_close\": \"CLOSE\","
        "\"payload_stop\": \"STOP\","
        "\"state_opening\": \"OPENING\","
        "\"state_closing\": \"CLOSING\","
        "\"state_stopped\": \"STOPPED\","
        "\"optimistic\": false,"
        "\"retain\": true"
        "}",
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data());

    return required >= 0 && static_cast<size_t>(required) < bufferLength;        
}

bool CoverDevice::BuildStateMessages(StateMessageList& list) const noexcept
{
    if (!m_tapOpen || !m_tapClose)
        return false;

    std::string_view id = GetId();
    const char* payload = m_tapOpen->GetStateAs<DigitalValue>()
        ? "OPENING"
        : m_tapClose->GetStateAs<DigitalValue>()
            ? "CLOSING"
            : "STOPPED";

    StateMessage message{};
    snprintf(message.Topic, sizeof(message.Topic), "domo/dev/%.*s/state", (int)id.size(), id.data());
    snprintf(message.Payload, sizeof(message.Payload), payload);
    message.Retain = false;
    return list.Add(message);
}

void CoverDevice::SubscribeToStateChanges() noexcept
{
    if (m_tapOpen && m_tapClose)
        return; // Already subscribed

    auto filter = TryGetFilter();
    if (!filter)
        return;

    auto openPin = filter->GetOpenOutputPin(),
         closePin = filter->GetCloseOutputPin();

    m_tapOpen = PinFactory::CreateInputPin<DigitalValue>(this);
    m_tapClose = PinFactory::CreateInputPin<DigitalValue>(this);
    Pin::Connect(m_tapOpen, openPin);
    Pin::Connect(m_tapClose, closePin);
}

void CoverDevice::ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept
{
    if (subtopic != "action")
        return;

    if (auto filter = TryGetFilter())
    {
        if (command == "OPEN")
            filter->Open();
        else if (command == "CLOSE")
            filter->Close();
        else
            filter->Stop();
    }
}

void CoverDevice::OnPinStateChanged(const Pin& pin) noexcept
{
    if (!m_tapOpen || !m_tapClose)
        return;

    auto eventBus = TryGetEventBus();
    if (!eventBus)
        return;

    std::string_view id = GetId();
    const char* payload = m_tapOpen->GetStateAs<DigitalValue>()
        ? "OPENING"
        : m_tapClose->GetStateAs<DigitalValue>()
            ? "CLOSING"
            : "STOPPED";

    BridgeEvent event{};
    event.Type = BridgeEvent::Type::PublishState;
    event.TopicLength = snprintf(event.Topic, sizeof(event.Topic), "domo/dev/%.*s/state", (int)id.size(), id.data());
    event.PayloadLength = snprintf(event.Payload, sizeof(event.Payload), payload);
    event.Retain = false;
    eventBus->EnqueueEvent(event);
}
