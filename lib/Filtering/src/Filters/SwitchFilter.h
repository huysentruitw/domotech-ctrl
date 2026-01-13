#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class SwitchFilter final : public Filter
{
public:
    SwitchFilter(std::string_view id = {});

    void SetState(DigitalValue state);
    bool SetStateCallback(const std::function<void(SwitchFilter&, DigitalValue)>& callback);

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_feedbackInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;

    std::function<void(SwitchFilter&, DigitalValue)> m_stateCallback;
};
