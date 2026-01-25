#include "DigitalPassthroughFilter.h"

#include <PinFactory.h>

DigitalPassthroughFilter::DigitalPassthroughFilter(std::string_view id) noexcept
    : Filter(FilterType::DigitalPassthrough, id)
{
    m_inputPin = PinFactory::CreateInputPin<DigitalValue>("Input", this);

    m_outputPin = PinFactory::CreateOutputPin<DigitalValue>("Output");

    m_inputPins = { m_inputPin };
    m_outputPins = { m_outputPin };
}

void DigitalPassthroughFilter::OnPinStateChanged(const Pin& pin) noexcept
{
    if (pin == m_inputPin)
    {
        const auto state = pin.GetStateAs<DigitalValue>();
        m_outputPin->SetState(state);
    }
}
