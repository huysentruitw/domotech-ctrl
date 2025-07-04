#include "ModuleBase.h"

ModuleBase::ModuleBase(uint8_t address, ModuleType type)
    : address(address), type(type)
{
}

uint8_t ModuleBase::GetAddress()
{
    return this->address;
}

ModuleType ModuleBase::GetType()
{
    return this->type;
}

void ModuleBase::Process()
{
    // Default behavior (can be overridden by derived classes)
}
