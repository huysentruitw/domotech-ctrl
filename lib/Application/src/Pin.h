#pragma once

#include "DigitalValue.h"
#include "DimmerControlValue.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class PinDirection
{
    Input,
    Output
};

typedef std::variant<DigitalValue, DimmerControlValue> PinState;

class Pin
{
public:
    Pin(
        const PinDirection direction,
        const PinState defaultState,
        const std::optional<std::function<void(const Pin&)>> onStateChange = std::nullopt,
        const std::string name = "");

    PinDirection GetDirection() const;
    const PinState& GetState() const;
    template<typename TState> TState GetStateAs() const { return std::get<TState>(m_state); }
    void SetState(const PinState& newState);

    static bool Connect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin);
    static bool Disconnect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin);

private:
    const PinDirection m_direction;
    const PinState m_defaultState;
    PinState m_state;
    const std::optional<std::function<void(const Pin&)>> m_onStateChange;
    const std::string m_name;

    std::weak_ptr<Pin> m_connectedOutputPin;
    std::vector<std::weak_ptr<Pin>> m_connectedInputPins;

    void NotifyConnectedInputPins(const PinState& newState);
};
