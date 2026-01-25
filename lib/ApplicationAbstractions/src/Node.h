#pragma once

#include "Pin.h"

#include <memory>
#include <span>
#include <string>
#include <vector>

class Node
{
public:
    const std::string& GetId() const noexcept { return m_id; }

    std::span<const std::weak_ptr<Pin>> GetInputPins() const noexcept
    {
        return m_inputPins;
    }

    std::span<const std::weak_ptr<Pin>> GetOutputPins() const noexcept
    {
        return m_outputPins;
    }

    std::shared_ptr<Pin> TryGetPinByName(PinDirection pinDirection, std::string_view name) const noexcept
    {
        const auto& container = pinDirection == PinDirection::Input ? m_inputPins : m_outputPins;
        for (const auto& weak : container)
            if (auto pin = weak.lock(); pin && pin->GetName() == name)
                return pin;

        return nullptr;
    }

protected:
    std::vector<std::weak_ptr<Pin>> m_inputPins;
    std::vector<std::weak_ptr<Pin>> m_outputPins;
    void SetId(std::string_view id) noexcept
    {
        m_id = std::string(id);
    }

private:
    std::string m_id;
};
