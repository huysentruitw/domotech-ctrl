#pragma once

#include "FilterBase.h"

#include <memory>
#include <vector>

class DimmerFilter : public FilterBase
{
public:
    DimmerFilter();

    std::vector<std::weak_ptr<Pin>> GetInputPins() const override;
    std::vector<std::weak_ptr<Pin>> GetOutputPins() const override;

private:
    std::shared_ptr<Pin> m_dimmerInputPin;
    std::shared_ptr<Pin> m_dimmerOutputPin;
};
