#pragma once

#include "DigitalValue.h"
#include "DimmerControlValue.h"
#include "IPinObserver.h"
#include "TemperatureValue.h"

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
    TemperatureValue> PinState;

constexpr const char* PinStateTypes[] = {
    "DigitalValue",
    "DimmerControlValue",
    "TemperatureValue"
};

class Pin final
{
public:
    Pin(const PinDirection direction, const PinState defaultState, IPinObserver* observer = nullptr) noexcept;

    ~Pin() noexcept;

    std::string_view GetName() const noexcept;
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

    bool operator==(const std::shared_ptr<Pin>& other) const noexcept
    {
        return this == other.get();
    }

private:
    const PinDirection m_direction;
    const PinState m_defaultState;
    PinState m_state;
    IPinObserver* m_observer = nullptr;

    std::string_view m_name;

    std::weak_ptr<Pin> m_connectedOutputPin;
    std::vector<std::weak_ptr<Pin>> m_connectedInputPins;

    void NotifyConnectedInputPins(const PinState& newState) noexcept;
};

template<typename TContainer>
int8_t FindIndex(const Pin& pin, const TContainer& list) noexcept
{
    for (uint8_t i = 0; i < list.size(); ++i)
        if (pin == list[i])
            return i;

    return -1;
}
