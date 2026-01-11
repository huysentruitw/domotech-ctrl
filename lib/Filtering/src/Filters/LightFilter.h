#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class LightFilter final : public Filter
{
public:
    LightFilter();

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_feedbackInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;
};
