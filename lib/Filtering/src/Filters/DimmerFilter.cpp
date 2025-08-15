#include "DimmerFilter.h"

#include <PinFactory.h>

DimmerFilter::DimmerFilter()
    : Filter(FilterType::Dimmer)
{
    m_toggleInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Toggle",
        [this](const Pin& pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
            {
                const auto currentState = m_controlOutputPin->GetStateAs<DimmerControlValue>();
                const auto newState = currentState.GetPercentage() == 0 ? DimmerControlValue(100, 2) : DimmerControlValue(0, 2);
                m_controlOutputPin->SetState(newState);
            }
        });

    m_controlOutputPin = PinFactory::CreateOutputPin<DimmerControlValue>("Control");
}

std::vector<std::weak_ptr<Pin>> DimmerFilter::GetInputPins() const
{
    return { m_toggleInputPin };
}

std::vector<std::weak_ptr<Pin>> DimmerFilter::GetOutputPins() const
{
    return { m_controlOutputPin };
}
