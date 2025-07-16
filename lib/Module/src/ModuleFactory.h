#pragma once

#include <cstdint>
#include <memory>

#include "ModuleBase.h"

class ModuleFactory final
{
public:
    ModuleFactory() = delete; // Prevent instantiation

    static std::unique_ptr<ModuleBase> CreateModule(const ModuleType type, const uint8_t address, const uint16_t data);
};
