#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class DimmerFilter final : public Filter
{
public:
    DimmerFilter(std::string_view id = {}) noexcept;

    void SetState(DimmerControlValue state) noexcept;
    bool SetStateCallback(const std::function<void(const DimmerFilter&, DimmerControlValue)>& callback) noexcept;

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;

    std::function<void(const DimmerFilter&, DimmerControlValue)> m_stateCallback;
};
