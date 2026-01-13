#pragma once

#include "Client.h"
#include "EventLoop.h"
#include "Processor.h"

#include <Bridge.h>
#include <Filter.h>

#include <string>

class HomeAssistantBridge final : public Bridge
{
public:
    HomeAssistantBridge(const char* uri, const char* username, const char* password);

    void Init();
    void RegisterFilter(std::weak_ptr<Filter> filter) override;

private:
    Processor m_processor;
    EventLoop m_eventLoop;
    Client m_client;
};
