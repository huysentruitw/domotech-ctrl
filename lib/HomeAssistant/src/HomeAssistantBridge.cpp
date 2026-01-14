#include "HomeAssistantBridge.h"

#include <Filters/SwitchFilter.h>
#include <Filters/LightFilter.h>
#include <PinFactory.h>

#include <algorithm>

#ifndef NATIVE_BUILD
    #include "esp_log.h"
#else
    #define ESP_LOGI(...) {}
#endif

#define TAG "HA_BRIDGE"

HomeAssistantBridge::HomeAssistantBridge(const char* uri, const char* username, const char* password) noexcept
    : m_processor(m_client, m_eventLoop)
    , m_eventLoop(m_processor)
    , m_client(uri, username, password, EventLoop::ForwardEvent, &m_eventLoop)
{
}

void HomeAssistantBridge::Init() noexcept
{
    ESP_LOGI(TAG, "Init");
    m_client.Connect();
    m_eventLoop.Start();
}

void HomeAssistantBridge::RegisterFilter(std::weak_ptr<Filter> filter) noexcept
{
    m_processor.RegisterFilter(filter);
}

void HomeAssistantBridge::UnregisterFilter(std::weak_ptr<Filter> filter) noexcept
{
    m_processor.UnregisterFilter(filter);
}
