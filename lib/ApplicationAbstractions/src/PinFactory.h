#pragma once

#include "IPinObserver.h"
#include "Pin.h"

#include <memory>

class PinFactory final
{
public:
    template<typename TState> static const std::shared_ptr<Pin> CreateInputPin(IPinObserver* observer = nullptr, const TState& defaultState = TState()) noexcept
    {
        return std::make_shared<Pin>(PinDirection::Input, defaultState, observer);
    }

    template<typename TState> static const std::shared_ptr<Pin> CreateInputPin(std::string_view name, IPinObserver* observer = nullptr, const TState& defaultState = TState()) noexcept
    {
        const auto pin = std::make_shared<Pin>(PinDirection::Input, defaultState, observer);
        pin->SetName(name);
        return pin;
    }

    template<typename TState> static const std::shared_ptr<Pin> CreateOutputPin(const TState& defaultState = TState()) noexcept
    {
        return std::make_shared<Pin>(PinDirection::Output, defaultState);
    }

    template<typename TState> static const std::shared_ptr<Pin> CreateOutputPin(std::string_view name, const TState& defaultState = TState()) noexcept
    {
        const auto pin = std::make_shared<Pin>(PinDirection::Output, defaultState);
        pin->SetName(name);
        return pin;
    }
};
