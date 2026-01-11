#pragma once

#include "Pin.h"

#include <memory>
#include <functional>

class PinFactory final
{
public:
    template<typename TState> static const std::shared_ptr<Pin> CreateInputPin(const std::function<void(const Pin&)> onStateChange = {}, const TState& defaultState = TState())
    {
        return std::make_shared<Pin>(PinDirection::Input, defaultState, onStateChange);
    }

    template<typename TState> static const std::shared_ptr<Pin> CreateInputPin(std::string_view name, const std::function<void(const Pin&)> onStateChange = {}, const TState& defaultState = TState())
    {
        const auto pin = std::make_shared<Pin>(PinDirection::Input, defaultState, onStateChange);
        pin->SetName(name);
        return pin;
    }

    template<typename TState> static const std::shared_ptr<Pin> CreateOutputPin(const TState& defaultState = TState())
    {
        return std::make_shared<Pin>(PinDirection::Output, defaultState);
    }

    template<typename TState> static const std::shared_ptr<Pin> CreateOutputPin(std::string_view name, const TState& defaultState = TState())
    {
        const auto pin = std::make_shared<Pin>(PinDirection::Output, defaultState);
        pin->SetName(name);
        return pin;
    }
};
