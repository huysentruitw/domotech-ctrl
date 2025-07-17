#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Pins.h"

class Node
{
public:    
    virtual std::vector<std::weak_ptr<InputPin<bool>>> GetDigitalInputPins() const { return {}; }
    virtual std::vector<std::weak_ptr<OutputPin<bool>>> GetDigitalOutputPins() const { return {}; }
};
