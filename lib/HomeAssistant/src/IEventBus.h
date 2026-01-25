#pragma once

#include "BridgeEvent.h"

class IEventBus
{
public:
    virtual ~IEventBus() noexcept = default;

    virtual void EnqueueEvent(const BridgeEvent& event) noexcept = 0;
};
