#include "LightDevice.h"
#include "IdSanitizer.h"

LightDevice::LightDevice(const std::shared_ptr<LightFilter>& filter) noexcept
    : Device(filter)
{
}

size_t LightDevice::BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength, "homeassistant/light/%.*s/config", (int)id.length(), id.data());
}

size_t LightDevice::BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    return snprintf(buffer, bufferLength,
        "{"
        "\"unique_id\": \"%.*s\","
        "\"name\": \"%.*s\","
        "\"state_topic\": \"domo/dev/%.*s/status\","
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
}

void LightDevice::ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept
{
    if (subtopic != "switch")
        return;

    const auto state = command == "ON" ? DigitalValue(true) : DigitalValue(false);
    if (auto filter = TryGetFilter())
        filter->SetState(state);
}

void LightDevice::SetStateChangedCallback(std::function<void(PinState)> callback) const noexcept
{
    if (auto filter = TryGetFilter())
    {
        filter->SetStateChangedCallback(
            [callback](const LightFilter& sender, DigitalValue state)
            {
                callback(state);
            });
    }
}
