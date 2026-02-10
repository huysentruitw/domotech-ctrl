#pragma once

#include <Filters/ClimateFilter.h>

#include "Device.h"

#include <memory>

class ClimateDevice final : public Device<ClimateFilter>
{
public:
    explicit ClimateDevice(const std::shared_ptr<ClimateFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept;

    size_t BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    size_t BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;

    void SubscribeToStateChanges() noexcept override;
    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;
    void EnqueueCurrentState() noexcept override;
};
