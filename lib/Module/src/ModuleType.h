#pragma once

#include <string>

enum class ModuleType
{
    Dimmer = 3,
    Temperature = 4,
    Audio = 9,

    PushButton = 7,
    PushButtonWithIr = 6,
    PushButtonWithTemperature = 11,
    PushButtonWithLed = 12,

    Relais = 10,
    Teleruptor = 8,

    Input = 13,
};

std::string_view GetModuleTypeName(ModuleType moduleType) noexcept;
