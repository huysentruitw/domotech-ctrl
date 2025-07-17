#include "ModuleBase.h"

ModuleBase::ModuleBase(const Bus& bus, const uint8_t address, const ModuleType type)
    : bus(bus)
    , address(address)
    , type(type)
{
}

ScanResponse ModuleBase::Poll() const
{
    return this->bus.Poll(this->address);
}

ScanResponse ModuleBase::Exchange(const uint16_t data) const
{
    return this->bus.Exchange(this->address, data);
}

uint8_t ModuleBase::GetAddress() const
{
    return this->address;
}

ModuleType ModuleBase::GetType() const
{
    return this->type;
}
