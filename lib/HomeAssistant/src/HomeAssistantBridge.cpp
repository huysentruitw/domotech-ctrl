#include "HomeAssistantBridge.h"

#include "Devices/BinarySensorDevice.h"
#include "Devices/ClimateDevice.h"
#include "Devices/CoverDevice.h"
#include "Devices/DimmableLightDevice.h"
#include "Devices/LightDevice.h"
#include "Devices/SwitchDevice.h"
#include "IdSanitizer.h"

#include <Filters/ClimateFilter.h>
#include <Filters/DigitalPassthroughFilter.h>
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

void HomeAssistantBridge::Init(const char* uri, const char* username, const char* password) noexcept
{
    ESP_LOGI(TAG, "Init");
    m_eventLoop = std::make_shared<EventLoop>();
    m_client = std::make_shared<Client>(uri, username, password, EventLoop::ForwardEvent, m_eventLoop.get());
    m_processor = std::make_shared<Processor>(*m_client, *m_eventLoop);

    m_client->Connect();
    m_eventLoop->Start(m_processor);
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
        m_processor->RegisterDevice(std::make_shared<SwitchDevice>(switchFilter, m_eventLoop));
        return true;
    }

    if (filterType == FilterType::Light)
    {
        auto lightFilter = std::static_pointer_cast<LightFilter>(filterPtr);
        m_processor->RegisterDevice(std::make_shared<LightDevice>(lightFilter, m_eventLoop));
        return true;
    }

    if (filterType == FilterType::Dimmer)
    {
        auto dimmerFilter = std::static_pointer_cast<DimmerFilter>(filterPtr);
        m_processor->RegisterDevice(std::make_shared<DimmableLightDevice>(dimmerFilter, m_eventLoop));
        return true;
    }

    if (filterType == FilterType::Shutter)
    {
        auto shutterFilter = std::static_pointer_cast<ShutterFilter>(filterPtr);
        m_processor->RegisterDevice(std::make_shared<CoverDevice>(shutterFilter, m_eventLoop));
        return true;
    }

    if (filterType == FilterType::Climate)
    {
        auto climateFilter = std::static_pointer_cast<ClimateFilter>(filterPtr);
        m_processor->RegisterDevice(std::make_shared<ClimateDevice>(climateFilter, m_eventLoop));
        return true;
    }

    if (filterType == FilterType::DigitalPassthrough)
    {
        auto digitalPassthroughFilter = std::static_pointer_cast<DigitalPassthroughFilter>(filterPtr);
        m_processor->RegisterDevice(std::make_shared<BinarySensorDevice>(digitalPassthroughFilter, m_eventLoop));
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
    m_processor->UnregisterDevice(id);
    return true;
}
