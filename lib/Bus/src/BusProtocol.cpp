#include "BusProtocol.h"

#ifndef NATIVE_BUILD
#include "esp_rom_sys.h"
#endif

#define REPLY_BACKOFF_DELAY_US     500

BusProtocol::BusProtocol(BusDriver& driver)
    : driver(driver)
{
}

ScanResponse BusProtocol::Poll(const uint8_t address, const uint8_t retries)
{
    for (uint8_t i = 0; i <= retries; i++)
    {
        auto response = this->ExchangeInternal(address, 0, false);

        if (response.Success)
            return response;
    }

    return ScanResponse { .Success = false };
}

ScanResponse BusProtocol::Exchange(const uint8_t address, const uint16_t data, const uint8_t retries)
{
    for (uint8_t i = 0; i <= retries; i++)
    {
        auto response = this->ExchangeInternal(address, data, true);

        if (response.Success)
            return response;
    }

    return ScanResponse { .Success = false };
}

ScanResponse BusProtocol::ExchangeInternal(const uint8_t address, const uint16_t data, const bool forceDataExchange)
{
    this->driver.FlushInput();

    // Send module address
    uint8_t addressBytes[2] = { 0xC0, 0xE0 };

    addressBytes[0] |= (address & 0x0F);
    addressBytes[1] |= (address >> 4);
    if (!forceDataExchange) addressBytes[1] |= 0x10;

    this->driver.WriteBytes(addressBytes, 2);

    // Read module response
    uint8_t responseByte;
    if (!this->driver.ReadBytes(&responseByte, 1))
        return ScanResponse { .Success = false }; // Module didn't answer within timeout

    uint8_t parity1 = addressBytes[0] ^ addressBytes[1];

    // Short sequence ?
    if ((responseByte & 0xF0) == 0xD0)
    {
        return (responseByte & 0x0F) == (parity1 & 0x0F)
            ? ScanResponse { .Success = true }
            : ScanResponse { .Success = false };
    }

    // Long sequence ?
    if ((responseByte & 0xF0) == 0x80)
    {
        parity1 ^= responseByte;
        uint8_t moduleType = responseByte & 0x0F;

#ifndef NATIVE_BUILD
        esp_rom_delay_us(REPLY_BACKOFF_DELAY_US);
#endif

        uint8_t sendBuffer[4] = { 0x90, 0x90, 0x90, 0x90 };
        for (int i = 0; i < 4; i++)
        {
            sendBuffer[i] |= ((data >> (i * 4)) & 0x0F);
            parity1 ^= sendBuffer[i];
        }
        this->driver.WriteBytes(sendBuffer, 4);

        uint8_t receivedParity1;
        if (!this->driver.ReadBytes(&receivedParity1, 1))
            return ScanResponse { .Success = false }; // Module didn't answer within timeout

        uint8_t receiveBuffer[4];
        if (!this->driver.ReadBytes(receiveBuffer, 4))
            return ScanResponse { .Success = false }; // Module didn't answer within timeout

        uint8_t receivedParity2;
        if (!this->driver.ReadBytes(&receivedParity2, 1))
            return ScanResponse { .Success = false }; // Module didn't answer within timeout

        bool success = true;
        if (receivedParity1 != (0xB0 | (parity1 & 0x0F)))
            success = false;

        uint16_t responseData = 0;
        uint8_t parity2 = 0;
        for (int i = 0; i < 4; i++)
        {
            if ((receiveBuffer[i] & 0xF0) != 0xA0)
                success = false;

            responseData |= (((uint16_t)(receiveBuffer[i] & 0x0F)) << (i * 4));
            parity2 ^= receiveBuffer[i];
        }

        if (receivedParity2 != (0xB0 | (parity2 & 0x0F)))
            success = false;

        // Prepare parity2 for sending back to module
        parity2 &= 0x0F;
        parity2 |= 0xB0; 

#ifndef NATIVE_BUILD
        esp_rom_delay_us(REPLY_BACKOFF_DELAY_US);
#endif

        if (success)
        {
            this->driver.WriteBytes(&parity2, 1);
            return ScanResponse { .Success = true, .RespondedWithTypeAndData = true, .ModuleType = moduleType, .Data = responseData };
        }

        // Send incorrect parity so module knows there was data corruption
        parity2 ^= 0x0F;
        this->driver.WriteBytes(&parity2, 1);
        return ScanResponse { .Success = false };
    }

    return ScanResponse { .Success = false };
}