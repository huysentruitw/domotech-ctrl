#include "ModuleBase.h"

ModuleBase::ModuleBase(const Bus& bus, const uint8_t address, const ModuleType type)
    : m_bus(bus)
    , m_address(address)
    , m_type(type)
{
}

ScanResponse ModuleBase::Poll() const
{
    return m_bus.Poll(m_address);
}

ScanResponse ModuleBase::Exchange(const uint16_t data) const
{
    return m_bus.Exchange(m_address, data);
}

uint8_t ModuleBase::GetAddress() const
{
    return m_address;
}

ModuleType ModuleBase::GetType() const
{
    return m_type;
}
