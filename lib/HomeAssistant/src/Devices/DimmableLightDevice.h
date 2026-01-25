#pragma once

#include <Filters/DimmerFilter.h>
#include <IPinObserver.h>
#include <Pin.h>

#include "Device.h"

#include <memory>

class DimmableLightDevice final : public Device<DimmerFilter>, private IPinObserver
{
public:
    explicit DimmableLightDevice(const std::shared_ptr<DimmerFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept;

    size_t BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    size_t BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;

    void SubscribeToStateChanges() noexcept override;
    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;

private:
    mutable std::shared_ptr<Pin> m_tap;

    void OnPinStateChanged(const Pin& pin) noexcept override;
    static uint8_t ParsePercentage(std::string_view value) noexcept;
};
