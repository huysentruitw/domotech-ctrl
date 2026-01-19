#pragma once

#include <Filters/DigitalPassthroughFilter.h>

#include "Device.h"

#include <memory>

class BinarySensorDevice final : public Device<DigitalPassthroughFilter>
{
public:
    explicit BinarySensorDevice(const std::shared_ptr<DigitalPassthroughFilter>& filter) noexcept;

    size_t BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    size_t BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;

    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;
    void SetStateChangedCallback(std::function<void(PinState)> callback) const noexcept override;
};
