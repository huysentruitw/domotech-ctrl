#pragma once

#include <cstdint>
#include <memory>
#include <vector>

template<typename TState> class InputPin final
{
public:
    InputPin(const TState initialState = TState())
        : state(initialState)
    {
    }

    TState GetState() const { return this->state; }
    void SetState(const TState newState) { this->state = newState; }    

private:
    TState state;
};

template<typename TState> class OutputPin final
{
public:
    OutputPin(const TState initialState = TState())
        : state(initialState)
    {
    }

    TState GetState() const { return this->state; }
    void SetState(const TState newState) { this->state = newState; }

    void Connect(const std::weak_ptr<InputPin<TState>>& inputPin)
    {
        this->connectedInputs.push_back(inputPin);
        inputPin->SetState(this->state);
    }

    void Disconnect(const std::weak_ptr<InputPin<TState>>& inputPin)
    {
        auto it = std::remove_if(this->connectedInputs.begin(), this->connectedInputs.end(),
            [&inputPin](const std::weak_ptr<InputPin<TState>>& pin) { return pin.lock() == inputPin.lock(); });
        this->connectedInputs.erase(it, this->connectedInputs.end());
    }

private:
    TState state;
    std::vector<std::weak_ptr<InputPin<TState>>> connectedInputs;
};
