#pragma once

#include <memory>
#include <vector>
#include <Bus.h>
#include "ModuleBase.h"

class ModuleScanner final
{
public:
    ModuleScanner(Bus& bus);
    std::vector<std::unique_ptr<ModuleBase>> DetectModules();

private:
    Bus& m_bus;
};
