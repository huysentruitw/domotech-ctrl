#pragma once

#include "Client.h"
#include "EventLoop.h"
#include "Processor.h"

#include <Filter.h>
#include <IBridge.h>

class HomeAssistantBridge final : public IBridge
{
public:
    HomeAssistantBridge(const char* uri, const char* username, const char* password) noexcept;

    void Init() noexcept;
    bool RegisterAsDevice(std::weak_ptr<Filter> filter) noexcept override;
    bool UnregisterAsDevice(std::weak_ptr<Filter> filter) noexcept override;

private:
    Processor m_processor;
    EventLoop m_eventLoop;
    Client m_client;
};
