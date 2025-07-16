#pragma once

#include <stdint.h>
#include "BusDriver.h"
#include "ScanResponse.h"

class BusProtocol
{
public:
    BusProtocol(const BusDriver& driver);
    ScanResponse Poll(const uint8_t address, const uint8_t retries = 2) const;
    ScanResponse Exchange(const uint8_t address, const uint16_t data, const uint8_t retries = 2) const;

private:
    const BusDriver& driver;
    ScanResponse ExchangeInternal(const uint8_t address, const uint16_t data, const bool forceDataExchange) const;
};
