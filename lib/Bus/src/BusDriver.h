#pragma once

#include <cstdint>

class BusDriver
{
public:
    virtual void Init() const;
    virtual void WriteBytes(const uint8_t* data, const uint16_t len) const;
    virtual bool ReadBytes(uint8_t* data, const uint16_t len) const;
    virtual void FlushInput() const;
};
