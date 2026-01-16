#pragma once

#include <cstdint>
#include <vector>
#include <queue>

#include <Bus.h>
#include <ScanResponse.h>

class MockBus : public Bus
{
public:
    MockBus() noexcept
        : Bus(*static_cast<BusDriver*>(nullptr))  // The real bus driver is never used in the mock
    {
    }

    // Override the original methods to avoid using the actual driver
    ScanResponse Poll(const uint8_t address, const uint8_t retries = 2) const noexcept override
    {
        PollCalled = true;
        LastPolledAddress = address;
        LastPolledRetries = retries;
        
        return GetNextResponse();
    }

    ScanResponse Exchange(const uint8_t address, const uint16_t data, const uint8_t retries = 2) const noexcept override
    {
        ExchangeCalled = true;
        LastExchangeAddress = address;
        LastExchangeData = data;
        LastExchangeRetries = retries;
        
        return GetNextResponse();
    }

    // Helper method to queue responses for later consumption
    void QueueResponse(const ScanResponse& response) noexcept
    {
        ResponseQueue.push(response);
    }

    void ClearCalls() noexcept
    {
        PollCalled = false;
        ExchangeCalled = false;
        LastPolledAddress = 0;
        LastExchangeAddress = 0;
        LastExchangeData = 0;
        LastPolledRetries = 0;
        LastExchangeRetries = 0;
    }

private:
    ScanResponse GetNextResponse() const noexcept
    {
        if (ResponseQueue.empty()) {
            return { .Success = false };
        }
        
        ScanResponse response = ResponseQueue.front();
        ResponseQueue.pop();
        return response;
    }

public:
    // For test verification
    mutable bool PollCalled = false;
    mutable bool ExchangeCalled = false;
    mutable uint8_t LastPolledAddress = 0;
    mutable uint8_t LastExchangeAddress = 0;
    mutable uint16_t LastExchangeData = 0;
    mutable uint8_t LastPolledRetries = 0;
    mutable uint8_t LastExchangeRetries = 0;
    mutable std::queue<ScanResponse> ResponseQueue;
};
