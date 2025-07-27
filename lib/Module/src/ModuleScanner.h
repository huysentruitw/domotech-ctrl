#pragma once

#include <memory>
#include <vector>
#include <Bus.h>
#include "Module.h"

class ModuleScanner final
{
public:
    ModuleScanner(Bus& bus);
    std::vector<std::unique_ptr<Module>> DetectModules();

private:
    Bus& m_bus;
};
