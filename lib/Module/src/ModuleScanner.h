#pragma once

#include <Bus.h>

#include "Module.h"

#include <memory>
#include <vector>

class ModuleScanner final
{
public:
    ModuleScanner(Bus& bus) noexcept;
    std::vector<std::unique_ptr<Module>> DetectModules() const noexcept;

private:
    Bus& m_bus;
};
