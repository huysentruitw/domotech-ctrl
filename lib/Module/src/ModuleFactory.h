#pragma once

#include "Module.h"

#include <cstdint>
#include <memory>
#include <string_view>

class ModuleFactory final
{
public:
    ModuleFactory(Bus& bus);

    std::unique_ptr<Module> CreateModule(const ModuleType type, const uint8_t address, const uint16_t initialData) const noexcept;

private:
    Bus& m_bus;    
};
