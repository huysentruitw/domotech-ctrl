#pragma once

#include <Lock.h>
#include <StringHash.h>

#include "BridgeEvent.h"
#include "Client.h"
#include "Devices/Device.h"
#include "IEventBus.h"
#include "IEventProcessor.h"

#include <memory>
#include <queue>
#include <unordered_map>

class Processor final : public IEventProcessor
{
public:
    Processor(Client& client, IEventBus& eventBus) noexcept;

    void Process(const BridgeEvent& event) noexcept;

    void RegisterDevice(const std::shared_ptr<IDevice>& device) noexcept;
    void UnregisterDevice(std::string_view id) noexcept;

private:
    const Lock m_syncRoot;
    Client& m_client;
    IEventBus& m_eventBus;
    std::unordered_map<std::string, std::shared_ptr<IDevice>, StringHash, std::equal_to<>> m_devices;
    std::queue<std::string> m_discoveryQueue;

    void OnMqttConnected() noexcept;
    void OnMqttData(const BridgeEvent& event) noexcept;
    void OnCompleteDeviceRegistration(const BridgeEvent& event) noexcept;
    void OnUnregisterDevice(const BridgeEvent& event) noexcept;
    void OnPublishNextDiscovery() noexcept;
    void OnPublishState(const BridgeEvent& event) noexcept;
    void OnShutdown() noexcept;

    void PublishDeviceDiscovery(const IDevice& device) noexcept;
    void PublishDeviceRemoval(const IDevice& device) noexcept;

    std::shared_ptr<IDevice> TryGetDeviceById(std::string_view id) const noexcept;
};
