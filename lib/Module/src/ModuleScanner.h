#pragma once

#include <memory>
#include <vector>
#include <Bus.h>
#include "Module.h"

class ModuleScanner final
{
public:
    ModuleScanner(const Bus& bus);
    const std::vector<std::shared_ptr<Module>> DetectModules() const;

private:
    const Bus& m_bus;
};
