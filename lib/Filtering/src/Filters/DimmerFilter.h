#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class DimmerFilter : public Filter
{
public:
    DimmerFilter();

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
};
