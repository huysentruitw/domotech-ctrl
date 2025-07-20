#pragma once

#include "FilterBase.h"

#include <memory>
#include <vector>

class DimmerFilter : public FilterBase
{
public:
    DimmerFilter()
    {
        m_dimmerInputPin = std::make_shared<InputPin<DigitalValue>>(
            [this](DigitalValue value) {
                if (value) {
                    m_dimmerOutputPin->SetState(DimmerControlValue(100, 2));
                } else {
                    m_dimmerOutputPin->SetState(DimmerControlValue(0, 2));
                }
            },
            DigitalValue(false)
        );

        m_dimmerOutputPin = std::make_shared<OutputPin<DimmerControlValue>>(DimmerControlValue(0, 0));
    }

    std::vector<std::weak_ptr<InputPin<DigitalValue>>> GetDigitalInputPins() const override { return { m_dimmerInputPin }; }
    std::vector<std::weak_ptr<OutputPin<DimmerControlValue>>> GetDimmerControlOutputPins() const override { return {m_dimmerOutputPin}; }

private:
    std::shared_ptr<InputPin<DigitalValue>> m_dimmerInputPin;
    std::shared_ptr<OutputPin<DimmerControlValue>> m_dimmerOutputPin;
};
