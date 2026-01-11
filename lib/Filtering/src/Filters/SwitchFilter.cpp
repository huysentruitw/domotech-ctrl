#include "SwitchFilter.h"

#include <PinFactory.h>

SwitchFilter::SwitchFilter()
    : Filter(FilterType::Switch)
{
    m_toggleInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Toggle",
        [this](const Pin& pin) {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true)) {
                const auto newState = m_controlOutputPin->GetStateAs<DigitalValue>() == DigitalValue(true) ? DigitalValue(false) : DigitalValue(true);
                m_controlOutputPin->SetState(newState);
                m_feedbackOutputPin->SetState(newState);
            }
        });

    m_controlOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Control");
    m_feedbackOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Feedback");

    m_inputPins = { m_toggleInputPin };
    m_outputPins = { m_controlOutputPin, m_feedbackOutputPin };
}
