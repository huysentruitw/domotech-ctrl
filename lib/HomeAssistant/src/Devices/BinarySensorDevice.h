#pragma once

#include <Filters/DigitalPassthroughFilter.h>
#include <IPinObserver.h>
#include <Pin.h>

#include "Device.h"

#include <memory>

class BinarySensorDevice final : public Device<DigitalPassthroughFilter>, private IPinObserver
{
public:
    explicit BinarySensorDevice(const std::shared_ptr<DigitalPassthroughFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept;

    bool BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    bool BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;
    bool BuildStateMessages(StateMessageList& list) const noexcept override;

    void SubscribeToStateChanges() noexcept override;
    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;

private:
    mutable std::shared_ptr<Pin> m_tap;

    void OnPinStateChanged(const Pin& pin) noexcept override;
};
