#include "ClimateDevice.h"
#include "IdSanitizer.h"

ClimateDevice::ClimateDevice(const std::shared_ptr<ClimateFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept
    : Device(filter, eventBus)
{
}

bool ClimateDevice::BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    int required = snprintf(buffer, bufferLength, "homeassistant/climate/%.*s/config", (int)id.length(), id.data());
    return required >= 0 && static_cast<size_t>(required) < bufferLength;
}

bool ClimateDevice::BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept
{
    std::string_view id = GetId();
    int required = snprintf(buffer, bufferLength,
        "{"
        "\"unique_id\": \"%.*s\","
        "\"name\": \"%.*s\","
        "\"current_temperature_topic\": \"domo/dev/%.*s/temp/current\","
        "\"modes\": [\"auto\",\"off\",\"heat\"],"
        "\"mode_state_topic\": \"domo/dev/%.*s/mode\","
        "\"mode_command_topic\": \"domo/dev/%.*s/mode/set\","
        "\"temperature_state_topic\": \"domo/dev/%.*s/temp\","
        "\"temperature_command_topic\": \"domo/dev/%.*s/temp/set\","
        "\"temperature_unit\": \"C\","
        "\"temp_step\": 0.5,"
        "\"max_temp\": 35.0,"
        "\"min_temp\": 7.0,"
        "\"precision\": 0.5,"
        "\"optimistic\": false,"
        "\"retain\": false"
        "}",
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data(),
        (int)id.length(), id.data());

    return required >= 0 && static_cast<size_t>(required) < bufferLength;
}

bool ClimateDevice::BuildStateMessages(StateMessageList& list) const noexcept
{
    return true;
}

void ClimateDevice::SubscribeToStateChanges() noexcept
{
}

void ClimateDevice::ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept
{
}

