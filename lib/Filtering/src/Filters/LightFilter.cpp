#include "LightFilter.h"

#include <PinFactory.h>

LightFilter::LightFilter(std::string_view id) noexcept
    : Filter(FilterType::Light, id)
{
    m_toggleInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Toggle",
        [this](const Pin& pin) {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true)) {
                const auto newState = m_controlOutputPin->GetStateAs<DigitalValue>() == DigitalValue(true) ? DigitalValue(false) : DigitalValue(true);
                SetState(newState);
            }
        });

    m_feedbackInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Feedback",
        [this](const Pin& pin) {
            const auto state = pin.GetStateAs<DigitalValue>();
            m_feedbackOutputPin->SetState(state);
            SetState(state);
        });

    m_controlOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Control");
    m_feedbackOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Feedback");

    m_inputPins = { m_toggleInputPin, m_feedbackInputPin };
    m_outputPins = { m_controlOutputPin, m_feedbackOutputPin };
}

void LightFilter::SetState(DigitalValue state) noexcept
{
    bool stateHasChanged = m_controlOutputPin->SetState(state);

    // If no feedback input is connected, use optimistic value
    if (!m_feedbackInputPin->IsConnected())
        m_feedbackOutputPin->SetState(state);

    if (m_stateCallback && stateHasChanged)
        m_stateCallback(*this, state);
}

bool LightFilter::SetStateCallback(const std::function<void(const LightFilter&, DigitalValue)>& callback) noexcept
{
    if (m_stateCallback)
        return false;

    m_stateCallback = callback;
    return true;
}
