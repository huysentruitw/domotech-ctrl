#pragma once

#include <cstdint>

class Node
{
    virtual uint8_t GetNumberOfDigitalInputPins() const { return 0; }
    virtual uint8_t GetNumberOfDigitalOutputPins() const { return 0; }
};
