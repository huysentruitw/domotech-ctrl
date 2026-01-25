#include "LightFilter.h"

#include <PinFactory.h>

LightFilter::LightFilter(std::string_view id) noexcept
    : Filter(FilterType::Light, id)
{
    m_toggleInputPin = PinFactory::CreateInputPin<DigitalValue>("Toggle", this);
    m_feedbackInputPin = PinFactory::CreateInputPin<DigitalValue>("Feedback", this);

    m_controlOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Control");
    m_feedbackOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Feedback");

    m_inputPins = { m_toggleInputPin, m_feedbackInputPin };
    m_outputPins = { m_controlOutputPin, m_feedbackOutputPin };
}

void LightFilter::SetState(DigitalValue state) noexcept
{
    m_controlOutputPin->SetState(state);

    // If no feedback input is connected, use optimistic value
    if (!m_feedbackInputPin->IsConnected())
        m_feedbackOutputPin->SetState(state);
}

void LightFilter::OnPinStateChanged(const Pin& pin) noexcept
{
    if (pin == m_toggleInputPin)
    {
        if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
        {
            const auto newState = m_controlOutputPin->GetStateAs<DigitalValue>() == DigitalValue(true) ? DigitalValue(false) : DigitalValue(true);
            SetState(newState);
        }        
    }
    else if (pin == m_feedbackInputPin)
    {
        const auto state = pin.GetStateAs<DigitalValue>();
        m_feedbackOutputPin->SetState(state);
        m_controlOutputPin->SetState(state);
    }
}