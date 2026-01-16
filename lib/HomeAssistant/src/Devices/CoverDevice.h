#pragma once

#include <Filters/ShutterFilter.h>

#include "Device.h"

#include <memory>

class CoverDevice final : public Device<ShutterFilter>
{
public:
    explicit CoverDevice(const std::shared_ptr<ShutterFilter>& filter) noexcept;

    size_t BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    size_t BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;

    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;
    void SetStateCallback(std::function<void(PinState)> callback) const noexcept override;
};
