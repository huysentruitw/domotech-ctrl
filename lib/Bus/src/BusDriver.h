#pragma once

#include <stdint.h>

class BusDriver
{
public:
    virtual void Init();
    virtual void WriteBytes(const uint8_t* data, const uint16_t len);
    virtual bool ReadBytes(uint8_t* data, const uint16_t len);
    virtual void FlushInput();
};
