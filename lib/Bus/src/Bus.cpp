#include "Bus.h"

#include <LockGuard.h>

#ifndef NATIVE_BUILD
 #include "esp_rom_sys.h"
 #define REPLY_BACKOFF() esp_rom_delay_us(500)
#else
 #define REPLY_BACKOFF()
#endif

Bus::Bus(const BusDriver& driver)
    : m_driver(driver)
    , m_syncRoot()
{
}

ScanResponse Bus::Poll(const uint8_t address, const uint8_t retries) const
{
    LockGuard guard(m_syncRoot);

    for (uint8_t i = 0; i <= retries; i++) {
        auto response = ExchangeInternal(address, 0, false);

        if (response.Success) {
            return response;
        }
    }

    return ScanResponse { .Success = false };
}

ScanResponse Bus::Exchange(const uint8_t address, const uint16_t data, const uint8_t retries) const
{
    LockGuard guard(m_syncRoot);

    for (uint8_t i = 0; i <= retries; i++) {
        auto response = ExchangeInternal(address, data, true);

        if (response.Success) {
            return response;
        }
    }

    return ScanResponse { .Success = false };
}

ScanResponse Bus::ExchangeInternal(const uint8_t address, const uint16_t data, const bool forceDataExchange) const
{
    m_driver.FlushInput();

    // Send module address
    uint8_t addressBytes[2] = { 0xC0, 0xE0 };

    addressBytes[0] |= (address & 0x0F);
    addressBytes[1] |= (address >> 4);
    if (!forceDataExchange) {
        addressBytes[1] |= 0x10;
    }

    m_driver.WriteBytes(addressBytes, 2);

    // Read module response
    uint8_t responseByte;
    if (!m_driver.ReadBytes(&responseByte, 1)) {
        return ScanResponse { .Success = false };
    }

    uint8_t parity1 = addressBytes[0] ^ addressBytes[1];

    // Short sequence ?
    if ((responseByte & 0xF0) == 0xD0) {
        return (responseByte & 0x0F) == (parity1 & 0x0F)
            ? ScanResponse { .Success = true }
            : ScanResponse { .Success = false };
    }

    // Long sequence ?
    if ((responseByte & 0xF0) == 0x80) {
        parity1 ^= responseByte;
        uint8_t moduleType = responseByte & 0x0F;

        REPLY_BACKOFF();

        uint8_t sendBuffer[4] = { 0x90, 0x90, 0x90, 0x90 };
        for (int i = 0; i < 4; i++) {
            sendBuffer[i] |= ((data >> (i * 4)) & 0x0F);
            parity1 ^= sendBuffer[i];
        }

        m_driver.WriteBytes(sendBuffer, 4);

        uint8_t receivedParity1;
        if (!m_driver.ReadBytes(&receivedParity1, 1)) {
            return ScanResponse { .Success = false };
        }

        uint8_t receiveBuffer[4];
        if (!m_driver.ReadBytes(receiveBuffer, 4)) {
            return ScanResponse { .Success = false };
        }

        uint8_t receivedParity2;
        if (!m_driver.ReadBytes(&receivedParity2, 1)) {
            return ScanResponse { .Success = false };
        }

        bool success = true;
        if (receivedParity1 != (0xB0 | (parity1 & 0x0F))) {
            success = false;
        }

        uint16_t responseData = 0;
        uint8_t parity2 = 0;
        for (int i = 0; i < 4; i++){
            if ((receiveBuffer[i] & 0xF0) != 0xA0) {
                success = false;
            }

            responseData |= (((uint16_t)(receiveBuffer[i] & 0x0F)) << (i * 4));
            parity2 ^= receiveBuffer[i];
        }

        if (receivedParity2 != (0xB0 | (parity2 & 0x0F))) {
            success = false;
        }

        // Prepare parity2 for sending back to module
        parity2 &= 0x0F;
        parity2 |= 0xB0; 

        REPLY_BACKOFF();

        if (success) {
            m_driver.WriteBytes(&parity2, 1);
            return ScanResponse { .Success = true, .RespondedWithTypeAndData = true, .ModuleType = moduleType, .Data = responseData };
        }

        // Send incorrect parity so module knows there was data corruption
        parity2 ^= 0x0F;
        m_driver.WriteBytes(&parity2, 1);
        return ScanResponse { .Success = false };
    }

    return ScanResponse { .Success = false };
}