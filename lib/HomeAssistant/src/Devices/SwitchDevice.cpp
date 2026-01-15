#include "SwitchDevice.h"
#include "IdSanitizer.h"

SwitchDevice::SwitchDevice(const std::shared_ptr<SwitchFilter>& filter) noexcept
    : Device(filter)
{
}

size_t SwitchDevice::BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength, "homeassistant/switch/%.*s/config", (int)id.length(), id.data());
}

size_t SwitchDevice::BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength,
        "{"
        "\"unique_id\": \"%.*s\","
        "\"name\": \"%.*s\","
        "\"state_topic\": \"domo/dev/%.*s/state\","
        "\"command_topic\": \"domo/dev/%.*s\","
        "\"payload_on\": \"ON\","
        "\"payload_off\": \"OFF\","
        "\"optimistic\": false,"
        "\"retain\": true"
        "}",
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data());
}

void SwitchDevice::ProcessCommand(std::string_view command) const noexcept
{
    const auto state = command == "ON" ? DigitalValue(true) : DigitalValue(false);
    if (auto filter = TryGetFilter())
        filter->SetState(state);
}

void SwitchDevice::SetStateCallback(std::function<void(PinState)> callback) const noexcept
{
    if (auto filter = TryGetFilter())
    {
        filter->SetStateCallback(
            [callback](const SwitchFilter& sender, DigitalValue state)
            {
                callback(state);
            });
    }
}
