#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class SwitchFilter final : public Filter
{
public:
    SwitchFilter();

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;
};
