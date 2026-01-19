#pragma once

#include <Filters/DimmerFilter.h>

#include "Device.h"

#include <memory>

class DimmableLightDevice final : public Device<DimmerFilter>
{
public:
    explicit DimmableLightDevice(const std::shared_ptr<DimmerFilter>& filter) noexcept;

    size_t BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    size_t BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;

    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;
    void SetStateChangedCallback(std::function<void(PinState)> callback) const noexcept override;

private:
    static uint8_t ParsePercentage(std::string_view value) noexcept;
};
