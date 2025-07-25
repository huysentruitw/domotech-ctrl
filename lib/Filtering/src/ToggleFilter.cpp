#include "ToggleFilter.h"

#include <PinFactory.h>

ToggleFilter::ToggleFilter()
{
    m_toggleInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Toggle",
        [this](const Pin& pin) {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true)) {
                const auto newState = m_controlOutputPin->GetStateAs<DigitalValue>() == DigitalValue(true) ? DigitalValue(false) : DigitalValue(true);
                m_controlOutputPin->SetState(newState);
            }
        });

    m_controlOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Control");
}

std::vector<std::weak_ptr<Pin>> ToggleFilter::GetInputPins() const
{
    return { m_toggleInputPin };
}

std::vector<std::weak_ptr<Pin>> ToggleFilter::GetOutputPins() const
{
    return { m_controlOutputPin };
}
