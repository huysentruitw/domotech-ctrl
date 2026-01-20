#include "Pin.h"

Pin::Pin(
    const PinDirection direction,
    const PinState defaultState,
    const std::function<void(const Pin&)> onStateChange) noexcept
    : m_direction(direction)
    , m_defaultState(defaultState)
    , m_state(defaultState)
    , m_onStateChange(onStateChange)
{
}

Pin::~Pin()
{
    for (auto it = m_connectedInputPins.begin(); it != m_connectedInputPins.end(); ++it)
    {
        if (auto input = it->lock())
        {
            input->m_connectedOutputPin.reset(); // Clear the connection
            input->SetState(input->m_defaultState);
        }
    }
}

std::string_view Pin::GetName() const noexcept
{
    return m_name;
}

void Pin::SetName(std::string_view name) noexcept
{
    m_name = name;
}

PinDirection Pin::GetDirection() const noexcept
{
    return m_direction;
}

const PinState& Pin::GetState() const noexcept
{
    return m_state;
}

bool Pin::SetState(const PinState& newState) noexcept
{
    if (newState == m_state)
        return false;

    m_state = newState;

    if (m_direction == PinDirection::Output)
        NotifyConnectedInputPins(newState);

    if (m_onStateChange)
        m_onStateChange(*this);

    return true;
}

bool Pin::Connect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin) noexcept
{
    const auto inputPinPtr = inputPin.lock();
    const auto outputPinPtr = outputPin.lock();

    if (!inputPinPtr || !outputPinPtr)
        return false; // Invalid pins

    if (!inputPinPtr->m_connectedOutputPin.expired())
        return false; // Input pin already connected to an output

    if (inputPinPtr->m_direction != PinDirection::Input || outputPinPtr->m_direction != PinDirection::Output)
        return false; // Invalid pin directions

    if (inputPinPtr->m_state.index() != outputPinPtr->m_state.index())
        return false; // State types do not match

    // Connect the pins
    inputPinPtr->m_connectedOutputPin = outputPin;
    outputPinPtr->m_connectedInputPins.push_back(inputPin);

    // Initialize state from output pin
    inputPinPtr->SetState(outputPinPtr->m_state);
    return true;
}

bool Pin::Disconnect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin) noexcept
{
    const auto inputPinPtr = inputPin.lock();
    const auto outputPinPtr = outputPin.lock();

    if (!inputPinPtr || !outputPinPtr)
        return false; // Invalid pins

    if (inputPinPtr->m_connectedOutputPin.lock() != outputPinPtr)
        return false; // Not connected

    // Remove the connection
    inputPinPtr->m_connectedOutputPin.reset();
    outputPinPtr->m_connectedInputPins.erase(
        std::remove_if(
            outputPinPtr->m_connectedInputPins.begin(),
            outputPinPtr->m_connectedInputPins.end(),
            [inputPinPtr](const std::weak_ptr<Pin>& p)
            {
                return p.lock() == inputPinPtr;
            }),
        outputPinPtr->m_connectedInputPins.end());

    // Reset state on disconnected
    inputPinPtr->SetState(inputPinPtr->m_defaultState);
    return true;
}

bool Pin::IsConnected() const noexcept
{
    if (!m_connectedOutputPin.expired())
        return true;

    for (const auto& pin : m_connectedInputPins)
    {
        if (!pin.expired())
            return true;
    }

    return false;
}

void Pin::NotifyConnectedInputPins(const PinState& newState) noexcept
{
    for (auto it = m_connectedInputPins.begin(); it != m_connectedInputPins.end();)
    {
        if (auto input = it->lock())
        {
            input->SetState(newState);
            ++it;
        }
        else
        {
            // Remove expired input pointers
            it = m_connectedInputPins.erase(it);
        }
    }
}
