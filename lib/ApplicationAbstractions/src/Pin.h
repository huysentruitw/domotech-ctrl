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

constexpr std::string_view PinStateTypes[] = {
    "DigitalValue",
    "DimmerControlValue"
};

class Pin final
{
public:
    Pin(
        const PinDirection direction,
        const PinState defaultState,
        const std::optional<std::function<void(const Pin&)>> onStateChange = std::nullopt);

    ~Pin();

    const std::string& GetName() const;
    void SetName(std::string_view name);

    PinDirection GetDirection() const;
    const PinState& GetState() const;
    template<typename TState> TState GetStateAs() const { return std::get<TState>(m_state); }
    bool SetState(const PinState& newState);

    static bool Connect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin);
    static bool Disconnect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin);
    bool IsConnected() const noexcept;

private:
    const PinDirection m_direction;
    const PinState m_defaultState;
    PinState m_state;
    const std::optional<std::function<void(const Pin&)>> m_onStateChange;
    std::string m_name;

    std::weak_ptr<Pin> m_connectedOutputPin;
    std::vector<std::weak_ptr<Pin>> m_connectedInputPins;

    void NotifyConnectedInputPins(const PinState& newState);
};
