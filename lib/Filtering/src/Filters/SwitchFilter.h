#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class SwitchFilter final : public Filter
{
public:
    SwitchFilter(std::string_view id = {});

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_feedbackInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;
};
