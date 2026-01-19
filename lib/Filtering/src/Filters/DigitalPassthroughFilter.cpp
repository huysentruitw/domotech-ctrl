#include "DigitalPassthroughFilter.h"

#include <PinFactory.h>

DigitalPassthroughFilter::DigitalPassthroughFilter(std::string_view id) noexcept
    : Filter(FilterType::DigitalPassthrough, id)
{
    m_inputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Input",
        [this](const Pin& pin)
        {
            auto state = pin.GetStateAs<DigitalValue>();
            bool stateHasChanged = m_outputPin->SetState(state);

            if (m_stateChangedCallback && stateHasChanged)
                m_stateChangedCallback(*this, state);
        });

    m_outputPin = PinFactory::CreateOutputPin<DigitalValue>("Output");

    m_inputPins = { m_inputPin };
    m_outputPins = { m_outputPin };
}

bool DigitalPassthroughFilter::SetStateChangedCallback(const std::function<void(const DigitalPassthroughFilter&, DigitalValue)>& callback) noexcept
{
    if (m_stateChangedCallback)
        return false;

    m_stateChangedCallback = callback;
    return true;
}
