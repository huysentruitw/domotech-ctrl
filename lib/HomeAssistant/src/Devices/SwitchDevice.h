#pragma once

#include <Filters/SwitchFilter.h>

#include "Device.h"

#include <memory>

class SwitchDevice final : public Device<SwitchFilter>
{
public:
    explicit SwitchDevice(const std::shared_ptr<SwitchFilter>& filter) noexcept;

    size_t BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    size_t BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;

    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;
    void SetStateChangedCallback(std::function<void(PinState)> callback) const noexcept override;
};
