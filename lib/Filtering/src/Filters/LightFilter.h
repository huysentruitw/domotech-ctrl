#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class LightFilter final : public Filter
{
public:
    LightFilter(std::string_view id = {}) noexcept;

    void SetState(DigitalValue state) noexcept;
    bool SetStateChangedCallback(const std::function<void(const LightFilter&, DigitalValue)>& callback) noexcept;

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_feedbackInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;

    std::function<void(const LightFilter&, DigitalValue)> m_stateChangedCallback;
};
