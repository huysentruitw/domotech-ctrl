#pragma once

#include "Pin.h"

#include <memory>
#include <optional>
#include <functional>

class PinFactory final
{
public:
    template<typename TState> static const std::shared_ptr<Pin> CreateInputPin(const std::optional<std::function<void(const Pin&)>> onStateChange = std::nullopt, const TState& defaultState = TState())
    {
        return std::make_shared<Pin>(PinDirection::Input, defaultState, onStateChange);
    }

    template<typename TState> static const std::shared_ptr<Pin> CreateOutputPin(const TState& defaultState = TState())
    {
        return std::make_shared<Pin>(PinDirection::Output, defaultState);
    }
};
