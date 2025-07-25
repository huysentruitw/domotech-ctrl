#include "Pin.h"

Pin::Pin(const PinDirection direction, const PinState defaultState, const std::optional<std::function<void(const Pin&)>> onStateChange)
    : m_direction(direction)
    , m_defaultState(defaultState)
    , m_state(defaultState)
    , m_onStateChange(onStateChange) {}

PinDirection Pin::GetDirection() const
{
    return m_direction;
}

const PinState& Pin::GetState() const
{
    return m_state;
}

void Pin::SetState(const PinState& newState)
{
    if (newState == m_state) {
        return;
    }

    m_state = newState;

    if (m_direction == PinDirection::Output) {
        NotifyConnectedInputPins(newState);
    }

    if (m_onStateChange.has_value()) {
        m_onStateChange.value()(*this);
    }
}

bool Pin::Connect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin)
{
    const auto inputPinPtr = inputPin.lock();
    const auto outputPinPtr = outputPin.lock();

    if (!inputPinPtr || !outputPinPtr) {
        // Invalid pins
        return false;
    }

    if (!inputPinPtr->m_connectedOutputPin.expired()) {
        // Input pin already connected to an output
        return false;
    }

    if (inputPinPtr->m_direction != PinDirection::Input || outputPinPtr->m_direction != PinDirection::Output) {
        // Invalid pin directions
        return false;
    }

    if (inputPinPtr->m_state.index() != outputPinPtr->m_state.index()) {
        // State types do not match
        return false;
    }

    // Connect the pins
    inputPinPtr->m_connectedOutputPin = outputPin;
    outputPinPtr->m_connectedInputPins.push_back(inputPin);

    // Initialize state from output pin
    inputPinPtr->SetState(outputPinPtr->m_state);
    return true;
}

bool Pin::Disconnect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin)
{
    const auto inputPinPtr = inputPin.lock();
    const auto outputPinPtr = outputPin.lock();

    if (!inputPinPtr || !outputPinPtr) {
        // Invalid pins
        return false;
    }

    if (inputPinPtr->m_connectedOutputPin.lock() != outputPinPtr) {
        // Not connected
        return false;
    }

    // Remove the connection
    inputPinPtr->m_connectedOutputPin.reset();
    outputPinPtr->m_connectedInputPins.erase(
        std::remove_if(outputPinPtr->m_connectedInputPins.begin(), outputPinPtr->m_connectedInputPins.end(),
                        [inputPinPtr](const std::weak_ptr<Pin>& p) { return p.lock() == inputPinPtr; }),
        outputPinPtr->m_connectedInputPins.end());

    // Reset state on disconnected
    inputPinPtr->SetState(inputPinPtr->m_defaultState);
    return true;
}

void Pin::NotifyConnectedInputPins(const PinState& newState)
{
    for (auto it = m_connectedInputPins.begin(); it != m_connectedInputPins.end();) {
        if (auto input = it->lock()) {
            input->SetState(newState);
            ++it;
        } else {
            // Remove expired input pointers
            it = m_connectedInputPins.erase(it);
        }
    }
}
