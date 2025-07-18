#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

// Forward declaration
template<typename TState> class OutputPin;

template<typename TState> class InputPin final : public std::enable_shared_from_this<InputPin<TState>>
{
public:
    InputPin(std::function<void(TState)> onStateChange, const TState initialState = TState())
        : m_onStateChange(onStateChange)
        , m_state(initialState) {
    }

    TState GetState() const {
        return m_state;
    }

    void SetState(const TState newState) {
        if (newState == m_state) {
            // No change, no need to update
            return;
        }

        m_state = newState;

        if (m_onStateChange) {
            m_onStateChange(newState);
        }
    }

    bool ConnectTo(std::weak_ptr<OutputPin<TState>> outputPin) {
        auto outputPinPtr = outputPin.lock();
        if (!outputPinPtr) {
            // Output pin is not valid
            return false;
        }

        if (!m_connectedOutputPin.expired()) {
            // Already connected to something
            return false;
        }

        m_connectedOutputPin = outputPin;
        outputPinPtr->m_connectedInputPins.push_back(this->shared_from_this());

        auto state = outputPinPtr->GetState();
        SetState(state); // Initialize state from output pin

        return true;
    }

    void Disconnect() {
        if (auto outputPin = m_connectedOutputPin.lock()) {
            auto& inputs = outputPin->m_connectedInputPins;
            inputs.erase(std::remove_if(inputs.begin(), inputs.end(),
                                         [this](const std::weak_ptr<InputPin<TState>>& input) {
                                             return input.lock() == this->shared_from_this();
                                         }), inputs.end());
            m_connectedOutputPin.reset();
        }

        // Reset state on disconnected
        SetState(TState());
    }

private:
    std::function<void(TState)> m_onStateChange;
    TState m_state;
    std::weak_ptr<OutputPin<TState>> m_connectedOutputPin;
};

template<typename TState> class OutputPin final
{
    friend class InputPin<TState>;

public:
    OutputPin(const TState initialState = TState()) : m_state(initialState) {
    }

    TState GetState() const {
        return m_state;
    }

    void SetState(const TState newState) { 
        if (newState == m_state) {
            // No change, no need to update
            return;
        }

        m_state = newState;

        // Notify all connected inputs
        for (auto it = m_connectedInputPins.begin(); it != m_connectedInputPins.end();) {
            if (auto input = it->lock()) {
                input->SetState(newState);
                ++it;
            } else {
                // Remove expired weak pointers
                it = m_connectedInputPins.erase(it);
            }
        }
    }

private:
    TState m_state;
    std::vector<std::weak_ptr<InputPin<TState>>> m_connectedInputPins;
};
