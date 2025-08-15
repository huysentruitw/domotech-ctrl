#pragma once

#include "Module.h"

#include <cstdint>
#include <memory>
#include <string>

class ModuleFactory final
{
public:
    ModuleFactory() = delete; // Prevent instantiation of static class

    static std::unique_ptr<Module> CreateModule(const Bus& bus, const ModuleType type, const uint8_t address, const uint16_t data);
};
