#include "DimmerFilter.h"

#include <PinFactory.h>

DimmerFilter::DimmerFilter(std::string_view id) noexcept
    : Filter(FilterType::Dimmer, id)
{
    m_toggleInputPin = PinFactory::CreateInputPin<DigitalValue>("Toggle", this);

    m_controlOutputPin = PinFactory::CreateOutputPin<DimmerControlValue>("Control");
    m_feedbackOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Feedback");

    m_inputPins = { m_toggleInputPin };
    m_outputPins = { m_controlOutputPin, m_feedbackOutputPin };
}

void DimmerFilter::Every100Ms() noexcept
{
    if (m_isPressed)
    {
        m_pressDurationInHundredMs++;

        // When press duration reaches 300ms (3 * 100ms), start dimming
        if (m_pressDurationInHundredMs >= 3 && !m_isLongPress)
        {
            m_isLongPress = true;
            m_isDimming = true;
        }

        // Handle dimming while button is held
        if (m_isDimming)
        {
            HandleDimming();
        }
    }
}

uint8_t DimmerFilter::GetLastOnPercentage() noexcept
{
    return m_lastOnPercentage;
}

void DimmerFilter::SetState(DimmerControlValue state) noexcept
{
    if (state.GetPercentage() > 0)
        m_lastOnPercentage = state.GetPercentage();

    m_controlOutputPin->SetState(state);

    // No feedback input, use optimistic value
    m_feedbackOutputPin->SetState(state.GetPercentage() == 0 ? DigitalValue(false) : DigitalValue(true));
}

void DimmerFilter::OnPinStateChanged(const Pin& pin) noexcept
{
    if (pin == m_toggleInputPin)
    {
        const auto buttonPressed = pin.GetStateAs<DigitalValue>() == DigitalValue(true);

        if (buttonPressed)
        {
            // Button pressed
            m_isPressed = true;
            m_pressDurationInHundredMs = 0;
            m_isLongPress = false;
            m_isDimming = false;
            // Default to dimming up
            m_dimmingDirection = 1;
        }
        else
        {
            // Button released
            if (m_isPressed)
            {
                if (!m_isLongPress && m_pressDurationInHundredMs < 3)
                {
                    // Short press - toggle on/off
                    const auto currentState = m_controlOutputPin->GetStateAs<DimmerControlValue>();
                    const auto newState = currentState.GetPercentage() == 0 ? DimmerControlValue(m_lastOnPercentage, 2) : DimmerControlValue(0, 2);
                    SetState(newState);
                }
                // If long press, just stop dimming (final state is already set)
            }

            // Reset press tracking
            m_isPressed = false;
            m_pressDurationInHundredMs = 0;
            m_isLongPress = false;
            m_isDimming = false;
        }
    }
}

void DimmerFilter::HandleDimming() noexcept
{
    const auto currentState = m_controlOutputPin->GetStateAs<DimmerControlValue>();
    uint8_t currentPercentage = currentState.GetPercentage();

    // Adjust brightness based on direction (1 for up, -1 for down)
    int newPercentageInt = static_cast<int>(currentPercentage) + m_dimmingDirection;

    // Clamp to valid range [0, 100]
    if (newPercentageInt < 0)
    {
        newPercentageInt = 0;
        // Optionally toggle direction at the boundary
        // m_dimmingDirection = 1;
    }
    else if (newPercentageInt > 100)
    {
        newPercentageInt = 100;
        // Optionally toggle direction at the boundary
        // m_dimmingDirection = -1;
    }

    uint8_t newPercentage = static_cast<uint8_t>(newPercentageInt);

    // Only send new state if percentage changed
    if (newPercentage != currentPercentage)
    {
        // Use 0 fade time for smooth dimming feedback during button hold
        SetState(DimmerControlValue(newPercentage, 0));
    }
}
