#pragma once

#include "Pin.h"

#include <memory>
#include <string>
#include <vector>

class Node
{
public:
    void SetName(const std::string name) { m_name = name; }
    const std::string& GetName() const { return m_name; }

    virtual std::vector<std::weak_ptr<Pin>> GetInputPins() const { return {}; }
    virtual std::vector<std::weak_ptr<Pin>> GetOutputPins() const { return {}; }

private:
    std::string m_name = "";
};
