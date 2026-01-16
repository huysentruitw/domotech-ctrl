#include "HomeAssistantBridge.h"

#include "Devices/ClimateDevice.h"
#include "Devices/CoverDevice.h"
#include "Devices/DimmableLightDevice.h"
#include "Devices/LightDevice.h"
#include "Devices/SwitchDevice.h"
#include "IdSanitizer.h"

#include <Filters/ClimateFilter.h>
#include <Filters/DimmerFilter.h>
#include <Filters/LightFilter.h>
#include <Filters/ShutterFilter.h>
#include <Filters/SwitchFilter.h>
#include <PinFactory.h>

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

bool HomeAssistantBridge::RegisterAsDevice(std::weak_ptr<Filter> filter) noexcept
{
    const auto filterPtr = filter.lock();
    if (!filterPtr)
        return false;

    const std::string id = IdSanitizer::Sanitize(filterPtr->GetId());
    ESP_LOGI(TAG, "RegisterAsDevice (Id: %.*s)", (int)id.length(), id.data());

    const auto filterType = filterPtr->GetType();
    if (filterType == FilterType::Switch)
    {
        auto switchFilter = std::static_pointer_cast<SwitchFilter>(filterPtr);
        m_processor.RegisterDevice(std::make_shared<SwitchDevice>(switchFilter));
        return true;
    }

    if (filterType == FilterType::Light)
    {
        auto lightFilter = std::static_pointer_cast<LightFilter>(filterPtr);
        m_processor.RegisterDevice(std::make_shared<LightDevice>(lightFilter));
        return true;
    }

    if (filterType == FilterType::Dimmer)
    {
        auto dimmerFilter = std::static_pointer_cast<DimmerFilter>(filterPtr);
        m_processor.RegisterDevice(std::make_shared<DimmableLightDevice>(dimmerFilter));
        return true;
    }

    if (filterType == FilterType::Shutter)
    {
        auto shutterFilter = std::static_pointer_cast<ShutterFilter>(filterPtr);
        m_processor.RegisterDevice(std::make_shared<CoverDevice>(shutterFilter));
        return true;
    }

    if (filterType == FilterType::Climate)
    {
        auto climateFilter = std::static_pointer_cast<ClimateFilter>(filterPtr);
        m_processor.RegisterDevice(std::make_shared<ClimateDevice>(climateFilter));
        return true;
    }

    return false;
}

bool HomeAssistantBridge::UnregisterAsDevice(std::weak_ptr<Filter> filter) noexcept
{
    const auto filterPtr = filter.lock();
    if (!filterPtr)
        return false;

    const std::string id = IdSanitizer::Sanitize(filterPtr->GetId());
    ESP_LOGI(TAG, "UnregisterAsDevice (Id: %.*s)", (int)id.length(), id.data());
    m_processor.UnregisterDevice(id);
    return true;
}
