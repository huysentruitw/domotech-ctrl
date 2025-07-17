#pragma once

#include <stdint.h>
#include <vector>

#include <BusDriver.h>

class MockBusDriver : public BusDriver
{
public:
    void Init() const override {}
    
    void WriteBytes(const uint8_t* data, const uint16_t len) const override
    {
        this->BytesWritten.insert(this->BytesWritten.end(), data, data + len);
    }
    
    bool ReadBytes(uint8_t* data, const uint16_t len) const override
    {
        if (this->BytesToRead.size() < len)
            return false; // Not enough data to read

        for (uint16_t i = 0; i < len; ++i)
        {
            data[i] = this->BytesToRead[i];
        }
        
        this->BytesToRead.erase(this->BytesToRead.begin(), this->BytesToRead.begin() + len);
        return true;
    }

    void FlushInput() const override
    {
        this->FlushInputCalled = true;
    }

public:
    mutable bool FlushInputCalled = false;
    mutable std::vector<uint8_t> BytesWritten;
    mutable std::vector<uint8_t> BytesToRead;
};
