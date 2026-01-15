#pragma once

#include <cstdint>

class BusDriver
{
public:
    virtual void Init() const noexcept;
    virtual void WriteBytes(const uint8_t* data, const uint16_t len) const noexcept;
    virtual bool ReadBytes(uint8_t* data, const uint16_t len) const noexcept;
    virtual void FlushInput() const noexcept;
};
