#pragma once

#include <Filters/SwitchFilter.h>
#include <IPinObserver.h>
#include <Pin.h>

#include "Device.h"

#include <memory>

class SwitchDevice final : public Device<SwitchFilter>, private IPinObserver
{
public:
    explicit SwitchDevice(const std::shared_ptr<SwitchFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept;

    size_t BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    size_t BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;

    void SubscribeToStateChanges() noexcept override;
    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;
    void EnqueueCurrentState() noexcept override;

private:
    mutable std::shared_ptr<Pin> m_tap;

    void OnPinStateChanged(const Pin& pin) noexcept override;
};
