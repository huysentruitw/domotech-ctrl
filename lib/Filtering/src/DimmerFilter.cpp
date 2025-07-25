#include "DimmerFilter.h"

#include <PinFactory.h>

DimmerFilter::DimmerFilter()
{
    m_dimmerInputPin = PinFactory::CreateInputPin<DigitalValue>(
        [this](const Pin& pin)
        {
            auto value = pin.GetStateAs<DigitalValue>();
            if (value == DigitalValue(true))
            {
                m_dimmerOutputPin->SetState(DimmerControlValue(100, 2));
            }
            else
            {
                m_dimmerOutputPin->SetState(DimmerControlValue(0, 2));
            }
        });

    m_dimmerOutputPin = PinFactory::CreateOutputPin<DimmerControlValue>();
}

std::vector<std::weak_ptr<Pin>> DimmerFilter::GetInputPins() const
{
    return { m_dimmerInputPin };
}

std::vector<std::weak_ptr<Pin>> DimmerFilter::GetOutputPins() const
{
    return { m_dimmerOutputPin };
}
