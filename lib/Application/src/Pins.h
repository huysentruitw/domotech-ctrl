#pragma once

#include <cstdint>
#include <memory>
#include <vector>

// Forward declaration
template<typename TState> class OutputPin;

template<typename TState> class InputPin final : public std::enable_shared_from_this<InputPin<TState>>
{
public:
    InputPin(const TState initialState = TState()) : state(initialState) {}

    TState GetState() const { return this->state; }
    void SetState(const TState newState)
    {
        if (newState == this->state)
            return; // No change, no need to update

        this->state = newState;
    }

    bool ConnectTo(std::weak_ptr<OutputPin<TState>> outputPin)
    {
        auto outputPinPtr = outputPin.lock();
        if (!outputPinPtr)
            return false; // Output pin is not valid

        if (!this->connectedOutputPin.expired())
            return false; // Already connected to something

        this->connectedOutputPin = outputPin;
        outputPinPtr->connectedInputPins.push_back(this->shared_from_this());

        auto state = outputPinPtr->GetState();
        this->SetState(state); // Initialize state from output pin
            
        return true;
    }

    void Disconnect()
    {
        if (auto outputPin = this->connectedOutputPin.lock())
        {
            auto& inputs = outputPin->connectedInputPins;
            inputs.erase(std::remove_if(inputs.begin(), inputs.end(),
                                         [this](const std::weak_ptr<InputPin<TState>>& input) {
                                             return input.lock() == this->shared_from_this();
                                         }), inputs.end());
            this->connectedOutputPin.reset();
        }

        this->state = TState(); // Reset state when disconnected
    }

private:
    TState state;
    std::weak_ptr<OutputPin<TState>> connectedOutputPin;
};

template<typename TState> class OutputPin final
{
    friend class InputPin<TState>;

public:
    OutputPin(const TState initialState = TState()) : state(initialState) {}

    TState GetState() const { return this->state; }
    void SetState(const TState newState)
    { 
        if (newState == this->state)
            return; // No change, no need to update

        this->state = newState;

        // Notify all connected inputs
        for (auto it = this->connectedInputPins.begin(); it != this->connectedInputPins.end();)
        {
            if (auto input = it->lock())
            {
                input->SetState(newState);
                ++it;
            }
            else
            {
                // Remove expired weak pointers
                it = this->connectedInputPins.erase(it);
            }
        }
    }

private:
    TState state;
    std::vector<std::weak_ptr<InputPin<TState>>> connectedInputPins;
};

template class InputPin<bool>;
template class OutputPin<bool>;
