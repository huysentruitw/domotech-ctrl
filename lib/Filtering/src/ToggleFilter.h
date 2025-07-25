#pragma once

#include "FilterBase.h"

#include <memory>
#include <vector>

class ToggleFilter : public FilterBase
{
public:
    ToggleFilter();

    std::vector<std::weak_ptr<Pin>> GetInputPins() const override;
    std::vector<std::weak_ptr<Pin>> GetOutputPins() const override;

private:
    std::shared_ptr<Pin> m_inputPin;
    std::shared_ptr<Pin> m_outputPin;
};
