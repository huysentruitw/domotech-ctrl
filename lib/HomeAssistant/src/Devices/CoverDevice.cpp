#include "CoverDevice.h"
#include "IdSanitizer.h"

CoverDevice::CoverDevice(const std::shared_ptr<ShutterFilter>& filter) noexcept
    : Device(filter)
{
}

size_t CoverDevice::BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength, "homeassistant/cover/%.*s/config", (int)id.length(), id.data());
}

size_t CoverDevice::BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength,
        "{"
        "\"unique_id\": \"%.*s\","
        "\"name\": \"%.*s\","
        "\"state_topic\": \"domo/dev/%.*s/status\","
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

void CoverDevice::SetStateChangedCallback(std::function<void(PinState)> callback) const noexcept
{
    if (auto filter = TryGetFilter())
    {
        filter->SetStateChangedCallback(
            [callback](const ShutterFilter& sender, ShutterControlValue state)
            {
                callback(state);
            });
    }
}
