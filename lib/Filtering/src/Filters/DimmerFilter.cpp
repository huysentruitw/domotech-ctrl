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
                m_feedbackOutputPin->SetState(newState.GetPercentage() == 0 ? DigitalValue(false) : DigitalValue(true));
            }
        });

    m_controlOutputPin = PinFactory::CreateOutputPin<DimmerControlValue>("Control");
    m_feedbackOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Feedback");

    m_inputPins = { m_toggleInputPin };
    m_outputPins = { m_controlOutputPin, m_feedbackOutputPin };
}
