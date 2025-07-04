#pragma once

enum class ModuleType
{
    Unknown = 0,
    
    Dimmer = 3,
    Temperature = 4,
    Audio = 9,

    PushButtons = 7,
    PushButtonsWithIr = 6,
    PushButtonsWithTemperature = 11,
    PushButtonsWithLeds = 12,

    Relais = 10,
    RelaisPulse = 8,

    Inputs = 13,
};
