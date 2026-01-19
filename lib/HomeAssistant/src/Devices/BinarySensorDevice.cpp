#include "BinarySensorDevice.h"
#include "IdSanitizer.h"

BinarySensorDevice::BinarySensorDevice(const std::shared_ptr<DigitalPassthroughFilter>& filter) noexcept
    : Device(filter)
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
        "\"state_topic\": \"domo/dev/%.*s/status\","
        "\"payload_on\": \"ON\","
        "\"payload_off\": \"OFF\","
        "\"optimistic\": false,"
        "\"retain\": true"
        "}",
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data());
}

void BinarySensorDevice::ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept
{
    // This device doesn't support commands
}

void BinarySensorDevice::SetStateChangedCallback(std::function<void(PinState)> callback) const noexcept
{
    if (auto filter = TryGetFilter())
    {
        filter->SetStateChangedCallback(
            [callback](const DigitalPassthroughFilter& sender, DigitalValue state)
            {
                callback(state);
            });
    }
}
