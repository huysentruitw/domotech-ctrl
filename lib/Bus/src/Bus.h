#pragma once

#include "BusDriver.h"
#include "ScanResponse.h"

#include <Lock.h>

#include <cstdint>

class Bus
{
public:
    Bus(const BusDriver& driver) noexcept;
    virtual ScanResponse Exchange(const uint8_t address, const uint16_t data, const bool forceDataExchange, const uint8_t retries = 2) const noexcept;

private:
    const BusDriver& m_driver;
    const Lock m_syncRoot;

    ScanResponse ExchangeInternal(const uint8_t address, const uint16_t data, const bool forceDataExchange) const noexcept;
};
