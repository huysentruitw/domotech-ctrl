#pragma once

#include "Pin.h"

#include <memory>
#include <span>
#include <string>
#include <vector>

class Node
{
public:
    void SetName(const std::string_view name) noexcept { m_name = name; }
    const std::string& GetName() const noexcept { return m_name; }

    std::span<const std::weak_ptr<Pin>> GetInputPins() const noexcept {
        return m_inputPins;
    }
    std::span<const std::weak_ptr<Pin>> GetOutputPins() const noexcept {
        return m_outputPins;
    }

protected:
    std::vector<std::weak_ptr<Pin>> m_inputPins;
    std::vector<std::weak_ptr<Pin>> m_outputPins;

private:
    std::string m_name;
};
