#include "ModuleType.h"

std::string_view GetModuleTypeName(ModuleType moduleType)
{
    using enum ModuleType;

    constexpr std::pair<ModuleType, std::string_view> identifiers[] = {
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

    for (auto&& [key, value] : identifiers)
        if (key == moduleType)
            return value;

    return "Unknown";
}
