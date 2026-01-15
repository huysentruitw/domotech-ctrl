#include "DimmerFilter.h"

#include <PinFactory.h>

DimmerFilter::DimmerFilter(std::string_view id) noexcept
    : Filter(FilterType::Dimmer, id)
{
    m_toggleInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Toggle",
        [this](const Pin& pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
            {
                const auto currentState = m_controlOutputPin->GetStateAs<DimmerControlValue>();
                const auto newState = currentState.GetPercentage() == 0 ? DimmerControlValue(100, 2) : DimmerControlValue(0, 2);
                SetState(newState);
            }
        });

    m_controlOutputPin = PinFactory::CreateOutputPin<DimmerControlValue>("Control");
    m_feedbackOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Feedback");

    m_inputPins = { m_toggleInputPin };
    m_outputPins = { m_controlOutputPin, m_feedbackOutputPin };
}

void DimmerFilter::SetState(DimmerControlValue state) noexcept
{
    bool stateHasChanged = m_controlOutputPin->SetState(state);

    // No feedback input, use optimistic value
    m_feedbackOutputPin->SetState(state.GetPercentage() == 0 ? DigitalValue(false) : DigitalValue(true));

    if (m_stateCallback && stateHasChanged)
        m_stateCallback(*this, state);
}

bool DimmerFilter::SetStateCallback(const std::function<void(const DimmerFilter&, DimmerControlValue)>& callback) noexcept
{
    if (m_stateCallback)
        return false;

    m_stateCallback = callback;
    return true;
}
