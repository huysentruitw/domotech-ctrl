#pragma once

#include <stdint.h>
#include "BusDriver.h"
#include "ScanResponse.h"

class BusProtocol
{
public:
    BusProtocol(BusDriver& driver);
    ScanResponse Poll(const uint8_t address, const uint8_t retries = 2);
    ScanResponse Exchange(const uint8_t address, const uint16_t data, const uint8_t retries = 2);

private:
    BusDriver& driver;
    ScanResponse ExchangeInternal(const uint8_t address, const uint16_t data, const bool forceDataExchange);
};
