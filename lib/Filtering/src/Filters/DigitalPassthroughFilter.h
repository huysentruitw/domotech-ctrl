#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class DigitalPassthroughFilter final : public Filter
{
public:
    DigitalPassthroughFilter(std::string_view id = {}) noexcept;

    bool SetStateCallback(const std::function<void(const DigitalPassthroughFilter&, DigitalValue)>& callback) noexcept;

private:
    std::shared_ptr<Pin> m_inputPin;
    std::shared_ptr<Pin> m_outputPin;

    std::function<void(const DigitalPassthroughFilter&, DigitalValue)> m_stateCallback;
};
