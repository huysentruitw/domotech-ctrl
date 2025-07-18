#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Pins.h"
#include "DigitalValue.h"

class Node
{
public:    
    virtual std::vector<std::weak_ptr<InputPin<DigitalValue>>> GetDigitalInputPins() const { return {}; }
    virtual std::vector<std::weak_ptr<OutputPin<DigitalValue>>> GetDigitalOutputPins() const { return {}; }
};
