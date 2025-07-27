#include "Module.h"

Module::Module(const Bus& bus, const uint8_t address, const ModuleType type)
    : m_bus(bus)
    , m_address(address)
    , m_type(type)
{
}

ScanResponse Module::Poll() const
{
    return m_bus.Poll(m_address);
}

ScanResponse Module::Exchange(const uint16_t data) const
{
    return m_bus.Exchange(m_address, data);
}

uint8_t Module::GetAddress() const
{
    return m_address;
}

ModuleType Module::GetType() const
{
    return m_type;
}
