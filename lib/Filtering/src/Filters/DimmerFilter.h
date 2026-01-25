#pragma once

#include <IPinObserver.h>

#include "Filter.h"

#include <memory>
#include <vector>

class DimmerFilter final : public Filter, private IPinObserver
{
public:
    DimmerFilter(std::string_view id = {}) noexcept;

    uint8_t GetLastOnPercentage() noexcept;
    void SetState(DimmerControlValue state) noexcept;

private:
    uint8_t m_lastOnPercentage = 100;

    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;

    void OnPinStateChanged(const Pin& pin) noexcept override;
};
