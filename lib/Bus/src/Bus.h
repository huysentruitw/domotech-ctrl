#pragma once

#include <cstdint>
#include "BusDriver.h"
#include "ScanResponse.h"

class Bus
{
public:
    Bus(const BusDriver& driver);
    virtual ScanResponse Poll(const uint8_t address, const uint8_t retries = 2) const;
    virtual ScanResponse Exchange(const uint8_t address, const uint16_t data, const uint8_t retries = 2) const;

private:
    const BusDriver& m_driver;
    ScanResponse ExchangeInternal(const uint8_t address, const uint16_t data, const bool forceDataExchange) const;
};
