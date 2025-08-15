#include "ModuleType.h"

const std::string GetModuleTypeName(ModuleType moduleType)
{
    static const std::unordered_map<ModuleType, std::string> identifiers = {
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

    return identifiers.contains(moduleType)
        ? identifiers.at(moduleType)
        : "Unknown";
}
