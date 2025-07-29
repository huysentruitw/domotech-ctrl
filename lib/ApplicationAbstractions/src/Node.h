#pragma once

#include "Pin.h"

#include <memory>
#include <vector>

class Node
{
public:
    virtual std::vector<std::weak_ptr<Pin>> GetInputPins() const { return {}; }
    virtual std::vector<std::weak_ptr<Pin>> GetOutputPins() const { return {}; }
};
