#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class DimmerFilter final : public Filter
{
public:
    DimmerFilter(std::string_view id = {}) noexcept;

    uint8_t GetLastOnPercentage() noexcept;
    void SetState(DimmerControlValue state) noexcept;
    bool SetStateChangedCallback(const std::function<void(const DimmerFilter&, DimmerControlValue)>& callback) noexcept;

private:
    uint8_t m_lastOnPercentage = 100;

    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;

    std::function<void(const DimmerFilter&, DimmerControlValue)> m_stateChangedCallback;
};
