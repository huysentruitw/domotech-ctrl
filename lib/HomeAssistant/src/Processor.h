#pragma once

#include <Lock.h>
#include <StringHash.h>

#include "BridgeEvent.h"
#include "Devices/Device.h"

#include <memory>
#include <unordered_map>

class Client; // forward declaration
class EventLoop;

class Processor final
{
    friend class EventLoop;

public:
    Processor(Client& client, EventLoop& eventLoop) noexcept;

    void RegisterDevice(const std::shared_ptr<IDevice>& device) noexcept;
    void UnregisterDevice(std::string_view id) noexcept;

private:
    const Lock m_syncRoot;
    Client& m_client;
    EventLoop& m_eventLoop;
    std::unordered_map<std::string, std::shared_ptr<IDevice>, StringHash, std::equal_to<>> m_devices;

    void Process(const BridgeEvent& event) noexcept;

    void OnMqttConnected() noexcept;
    void OnMqttData(const BridgeEvent& event) noexcept;
    void OnCompleteDeviceRegistration(const BridgeEvent& event) noexcept;
    void OnUnregisterDevice(const BridgeEvent& event) noexcept;
    void OnPublishState(const BridgeEvent& event) noexcept;
    void OnShutdown() noexcept;

    void PublishDeviceDiscovery(const IDevice& device) noexcept;
    void PublishDeviceRemoval(const IDevice& device) noexcept;

    std::shared_ptr<IDevice> TryGetDeviceById(std::string_view id) const noexcept;
};
