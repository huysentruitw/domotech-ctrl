#pragma once

#include <IPinObserver.h>

#include "Filter.h"

#include <memory>
#include <vector>

class DigitalPassthroughFilter final : public Filter, private IPinObserver
{
public:
    DigitalPassthroughFilter(std::string_view id = {}) noexcept;

    std::weak_ptr<Pin> GetInputPin() const noexcept { return m_inputPin; }
    std::weak_ptr<Pin> GetOutputPin() const noexcept { return m_outputPin; }

private:
    std::shared_ptr<Pin> m_inputPin;
    std::shared_ptr<Pin> m_outputPin;

    void OnPinStateChanged(const Pin& pin) noexcept override;
};
