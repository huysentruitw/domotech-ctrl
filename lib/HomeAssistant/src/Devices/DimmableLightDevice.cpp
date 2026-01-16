#include "DimmableLightDevice.h"
#include "IdSanitizer.h"

DimmableLightDevice::DimmableLightDevice(const std::shared_ptr<DimmerFilter>& filter) noexcept
    : Device(filter)
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
        "\"state_topic\": \"domo/dev/%.*s/status\","
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

uint8_t DimmableLightDevice::ParsePercentage(std::string_view value) noexcept
{
    uint32_t result = 0;

    for (char c : value)
    {
        if (c < '0' || c > '9')
            return 0;
        
        result = result * 10 + (c - '0');

        if (result > 100)
            return result;
    }

    return result;
}

void DimmableLightDevice::SetStateCallback(std::function<void(PinState)> callback) const noexcept
{
    if (auto filter = TryGetFilter())
    {
        filter->SetStateCallback(
            [this, callback](const DimmerFilter& sender, DimmerControlValue state)
            {
                callback(state);
            });
    }
}
