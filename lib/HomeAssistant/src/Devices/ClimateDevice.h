#pragma once

#include <Filters/ClimateFilter.h>

#include "Device.h"

#include <memory>

class ClimateDevice final : public Device<ClimateFilter>
{
public:
    ClimateDevice(std::string_view id, const std::weak_ptr<ClimateFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept;

    bool BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    bool BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;
    bool BuildStateMessages(StateMessageList& list) const noexcept override;

    void SubscribeToStateChanges() noexcept override;
    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;
};
