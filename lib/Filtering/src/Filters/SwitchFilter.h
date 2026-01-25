#pragma once

#include <IPinObserver.h>

#include "Filter.h"

#include <memory>
#include <vector>

class SwitchFilter final : public Filter, private IPinObserver
{
public:
    SwitchFilter(std::string_view id = {}) noexcept;

    void SetState(DigitalValue state) noexcept;

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_feedbackInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;

    void OnPinStateChanged(const Pin& pin) noexcept override;
};
