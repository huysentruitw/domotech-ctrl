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

                // If no feedback input is connected, use optimistic value
                if (!m_feedbackInputPin->IsConnected())
                    m_feedbackOutputPin->SetState(newState);
            }
        });

    m_feedbackInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Feedback",
        [this](const Pin& pin) {
            m_feedbackOutputPin->SetState(pin.GetStateAs<DigitalValue>());
        });

    m_controlOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Control");
    m_feedbackOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Feedback");

    m_inputPins = { m_toggleInputPin, m_feedbackInputPin };
    m_outputPins = { m_controlOutputPin, m_feedbackOutputPin };
}
