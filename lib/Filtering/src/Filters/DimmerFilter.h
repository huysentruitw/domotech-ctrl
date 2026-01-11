#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class DimmerFilter final : public Filter
{
public:
    DimmerFilter(std::string_view id = {});

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;
};
