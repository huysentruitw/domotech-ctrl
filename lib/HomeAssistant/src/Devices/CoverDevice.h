#pragma once

#include <Filters/ShutterFilter.h>
#include <IPinObserver.h>
#include <Pin.h>

#include "Device.h"

#include <memory>

class CoverDevice final : public Device<ShutterFilter>, private IPinObserver
{
public:
    explicit CoverDevice(const std::shared_ptr<ShutterFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept;

    size_t BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override;
    size_t BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override;

    void SubscribeToStateChanges() noexcept override;
    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override;
    void EnqueueCurrentState() noexcept override;

private:
    mutable std::shared_ptr<Pin> m_tapOpen;
    mutable std::shared_ptr<Pin> m_tapClose;

    void OnPinStateChanged(const Pin& pin) noexcept override;
};
