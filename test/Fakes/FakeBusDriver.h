#pragma once

#include <stdint.h>
#include <vector>

#include <BusDriver.h>

class FakeBusState
{
public:
    bool FlushInputCalled = false;
    std::vector<uint8_t> BytesWritten;
    std::vector<uint8_t> BytesToRead;
};

class FakeBusDriver : public BusDriver
{
public:
    FakeBusDriver(FakeBusState& state) : state(state) {}

    void Init() const override {}
    
    void WriteBytes(const uint8_t* data, const uint16_t len) const override
    {
        this->state.BytesWritten.insert(state.BytesWritten.end(), data, data + len);
    }
    
    bool ReadBytes(uint8_t* data, const uint16_t len) const override
    {
        if (this->state.BytesToRead.size() < len)
            return false; // Not enough data to read

        for (uint16_t i = 0; i < len; ++i)
        {
            data[i] = this->state.BytesToRead[i];
        }
        
        this->state.BytesToRead.erase(this->state.BytesToRead.begin(), this->state.BytesToRead.begin() + len);
        return true;
    }

    void FlushInput() const override
    {
        this->state.FlushInputCalled = true;
    }

private:
    FakeBusState& state;
};
