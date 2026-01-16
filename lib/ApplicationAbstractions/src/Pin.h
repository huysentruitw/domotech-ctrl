#pragma once

#include "DigitalValue.h"
#include "DimmerControlValue.h"
#include "ShutterControlValue.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

enum class PinDirection
{
    Input,
    Output
};

typedef std::variant<
    DigitalValue,
    DimmerControlValue,
    ShutterControlValue> PinState;

constexpr const char* PinStateTypes[] = {
    "DigitalValue",
    "DimmerControlValue",
    "ShutterControlValue",
};

class Pin final
{
public:
    Pin(
        const PinDirection direction,
        const PinState defaultState,
        const std::function<void(const Pin&)> onStateChange = {}) noexcept;

    ~Pin() noexcept;

    const std::string& GetName() const noexcept;
    void SetName(std::string_view name) noexcept;

    PinDirection GetDirection() const noexcept;
    const PinState& GetState() const noexcept;
    template<typename TState> TState GetStateAs() const noexcept
    {
        auto state = std::get_if<TState>(&m_state);
        return state ? *state : TState{};
    }

    bool SetState(const PinState& newState) noexcept;

    static bool Connect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin) noexcept;
    static bool Disconnect(std::weak_ptr<Pin> inputPin, std::weak_ptr<Pin> outputPin) noexcept;
    bool IsConnected() const noexcept;

private:
    const PinDirection m_direction;
    const PinState m_defaultState;
    PinState m_state;
    const std::function<void(const Pin&)> m_onStateChange;
    std::string m_name;

    std::weak_ptr<Pin> m_connectedOutputPin;
    std::vector<std::weak_ptr<Pin>> m_connectedInputPins;

    void NotifyConnectedInputPins(const PinState& newState) noexcept;
};
