#include "ModuleType.h"

constexpr auto& ModuleIdentifiers()
{
    using enum ModuleType;
    static constexpr std::pair<ModuleType, std::string_view> identifiers[] =
    {
        { ModuleType::Dimmer, "Dimmer" },
        { ModuleType::Temperature, "Temperature" },
        { ModuleType::Audio, "Audio" },

        { ModuleType::PushButton, "PushButton" },
        { ModuleType::PushButtonWithIr, "PushButtonWithIr" },
        { ModuleType::PushButtonWithTemperature, "PushButtonWithTemperature" },
        { ModuleType::PushButtonWithLed, "PushButtonWithLed" },

        { ModuleType::Relais, "Relais" },
        { ModuleType::Teleruptor, "Teleruptor" },

        { ModuleType::Input, "Input" },
    };
    return identifiers;
}

std::string_view GetModuleTypeName(ModuleType moduleType) noexcept
{
    for (auto&& [key, value] : ModuleIdentifiers())
    {
        if (key == moduleType)
            return value;
    }

    return "Unknown";
}

std::optional<ModuleType> GetModuleType(std::string_view typeName) noexcept
{
    for (auto&& [key, value] : ModuleIdentifiers())
    {
        if (value == typeName)
            return key;
    }

    return std::nullopt;
}
