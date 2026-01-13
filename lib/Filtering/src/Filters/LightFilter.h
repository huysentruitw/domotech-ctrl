#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class LightFilter final : public Filter
{
public:
    LightFilter(std::string_view id = {});

    void SetState(DigitalValue state);
    bool SetStateCallback(const std::function<void(LightFilter&, DigitalValue)>& callback);

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_feedbackInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;

    std::function<void(LightFilter&, DigitalValue)> m_stateCallback;
};
