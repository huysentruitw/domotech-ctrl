#pragma once

#include <memory>
#include <vector>
#include <BusProtocol.h>
#include "ModuleBase.h"

class ModuleScanner final
{
public:
    ModuleScanner(BusProtocol& bus);
    std::vector<std::unique_ptr<ModuleBase>> DetectModules();

private:
    BusProtocol& bus;
};
