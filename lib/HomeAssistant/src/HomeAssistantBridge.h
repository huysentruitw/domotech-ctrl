#pragma once

#include "Client.h"
#include "EventLoop.h"
#include "Processor.h"

#include <Filter.h>
#include <IBridge.h>

#include <memory>

class HomeAssistantBridge final : public IBridge
{
public:
    void Init(const char* uri, const char* username, const char* password) noexcept;
    bool RegisterAsDevice(std::weak_ptr<Filter> filter) noexcept override;
    bool UnregisterAsDevice(std::weak_ptr<Filter> filter) noexcept override;

private:
    std::shared_ptr<Processor> m_processor;
    std::shared_ptr<EventLoop> m_eventLoop;
    std::shared_ptr<Client> m_client;
};
