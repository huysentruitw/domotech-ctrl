#pragma once

#include <Lock.h>
#include <StringHash.h>

#include "BridgeEvent.h"
#include "Filter.h"

#include <memory>
#include <unordered_map>

class Client; // forward declaration
class EventLoop;

class Processor final
{
    friend class EventLoop;

public:
    Processor(Client& client, EventLoop& eventLoop) noexcept;

    void RegisterFilter(std::weak_ptr<Filter> filter) noexcept;

private:
    void SubscribeToStateChanges(std::shared_ptr<Filter> filter) noexcept;

private:
    const Lock m_syncRoot;
    Client& m_client;
    EventLoop& m_eventLoop;
    std::unordered_map<std::string, std::weak_ptr<Filter>, StringHash, std::equal_to<>> m_filters;

    void Process(const BridgeEvent& event) noexcept;

    void OnMqttConnected() noexcept;
    void OnMqttData(const BridgeEvent& event) noexcept;
    void OnCompleteFilterRegistration(const BridgeEvent& event) noexcept;
    void OnPublishState(const BridgeEvent& event) noexcept;
    void OnShutdown() noexcept;

    void PublishDeviceDiscovery(std::shared_ptr<Filter> filter) noexcept;
    void PublishFilterRemoval(std::string_view id) noexcept;

    std::shared_ptr<Filter> TryGetFilterById(std::string_view id) noexcept;
};
