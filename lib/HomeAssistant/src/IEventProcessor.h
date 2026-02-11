#pragma once

#include "BridgeEvent.h"

class IEventProcessor
{
public:
    virtual ~IEventProcessor() noexcept = default;

    virtual void Process(const BridgeEvent& event) noexcept = 0;
};
