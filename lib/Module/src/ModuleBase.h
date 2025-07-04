#pragma once

#include <stdint.h>
#include "ModuleType.h"

class ModuleBase
{
public:
    ModuleBase(uint8_t address, ModuleType moduleType);

    virtual void Process();

    uint8_t GetAddress();
    ModuleType GetType();

private:
    const uint8_t address;
    const ModuleType type;
};
