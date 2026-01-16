#pragma once

#include <cstdint>
#include <vector>

#include <BusDriver.h>

class MockBusDriver : public BusDriver
{
public:
    void Init() const noexcept override {}
    
    void WriteBytes(const uint8_t* data, const uint16_t len) const noexcept override
    {
        BytesWritten.insert(BytesWritten.end(), data, data + len);
    }
    
    bool ReadBytes(uint8_t* data, const uint16_t len) const noexcept override
    {
        if (BytesToRead.size() < len)
            return false; // Not enough data to read

        for (uint16_t i = 0; i < len; ++i)
            data[i] = BytesToRead[i];
        
        BytesToRead.erase(BytesToRead.begin(), BytesToRead.begin() + len);
        return true;
    }

    void FlushInput() const noexcept override
    {
        FlushInputCalled = true;
    }

public:
    mutable bool FlushInputCalled = false;
    mutable std::vector<uint8_t> BytesWritten;
    mutable std::vector<uint8_t> BytesToRead;
};
