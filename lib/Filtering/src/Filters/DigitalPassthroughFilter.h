#pragma once

#include <IPinObserver.h>

#include "Filter.h"

#include <memory>
#include <vector>

class DigitalPassthroughFilter final : public Filter, private IPinObserver
{
public:
    DigitalPassthroughFilter(std::string_view id = {}) noexcept;

private:
    std::shared_ptr<Pin> m_inputPin;
    std::shared_ptr<Pin> m_outputPin;

    void OnPinStateChanged(const Pin& pin) noexcept override;
};
