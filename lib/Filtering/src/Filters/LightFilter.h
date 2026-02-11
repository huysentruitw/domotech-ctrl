#pragma once

#include <IPinObserver.h>

#include "Filter.h"

#include <memory>
#include <vector>

class LightFilter final : public Filter, private IPinObserver
{
public:
    LightFilter(std::string_view id = {}) noexcept;

    void SetState(DigitalValue state) noexcept;

    std::weak_ptr<Pin> GetToggleInputPin() const noexcept { return m_toggleInputPin; }
    std::weak_ptr<Pin> GetFeedbackInputPin() const noexcept { return m_feedbackInputPin; }
    std::weak_ptr<Pin> GetControlOutputPin() const noexcept { return m_controlOutputPin; }
    std::weak_ptr<Pin> GetFeedbackOutputPin() const noexcept { return m_feedbackOutputPin; }

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_feedbackInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;

    void OnPinStateChanged(const Pin& pin) noexcept override;
};
