#include "ToggleFilter.h"

#include <PinFactory.h>

ToggleFilter::ToggleFilter()
{
    m_inputPin = PinFactory::CreateInputPin<DigitalValue>(
        [this](const Pin& pin) {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true)) {
                const auto newState = m_outputPin->GetStateAs<DigitalValue>() == DigitalValue(true) ? DigitalValue(false) : DigitalValue(true);
                m_outputPin->SetState(newState);
            }
        });

    m_outputPin = PinFactory::CreateOutputPin<DigitalValue>();
}

std::vector<std::weak_ptr<Pin>> ToggleFilter::GetInputPins() const
{
    return { m_inputPin };
}

std::vector<std::weak_ptr<Pin>> ToggleFilter::GetOutputPins() const
{
    return { m_outputPin };
}
