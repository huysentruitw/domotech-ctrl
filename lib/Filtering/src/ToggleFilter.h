#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class ToggleFilter : public Filter
{
public:
    ToggleFilter();

    const char* GetFilterName() const override { return "ToggleFilter"; }

    std::vector<std::weak_ptr<Pin>> GetInputPins() const override;
    std::vector<std::weak_ptr<Pin>> GetOutputPins() const override;

private:
    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
};
