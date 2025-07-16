#include "ModuleBase.h"

ModuleBase::ModuleBase(const uint8_t address, const ModuleType type)
    : address(address)
    , type(type)
{
}

ScanResponse ModuleBase::Poll(const BusProtocol& bus) const
{
    return bus.Poll(this->address);
}

ScanResponse ModuleBase::Exchange(const BusProtocol& bus, const uint16_t data) const
{
    return bus.Exchange(this->address, data);
}

uint8_t ModuleBase::GetAddress() const
{
    return this->address;
}

ModuleType ModuleBase::GetType() const
{
    return this->type;
}
