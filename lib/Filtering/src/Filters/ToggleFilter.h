#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class ToggleFilter : public Filter
{
public:
    ToggleFilter();

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;
};
