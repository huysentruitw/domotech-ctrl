#pragma once

#include "Module.h"

#include <cstdint>
#include <memory>
#include <string_view>

class ModuleFactory final
{
public:
    ModuleFactory() = delete; // Prevent instantiation of static class

    static std::unique_ptr<Module> CreateModule(Bus& bus, ModuleType type, uint8_t address, uint16_t initialData) noexcept;
};
