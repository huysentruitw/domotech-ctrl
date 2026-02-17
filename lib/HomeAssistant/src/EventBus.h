#pragma once

#include "IEventBus.h"
#include "IEventProcessor.h"

#ifndef NATIVE_BUILD

#include "freertos/FreeRTOS.h"

class EventBus final : public IEventBus
{
    friend class HomeAssistantBridge;

public:
    EventBus() noexcept;
    ~EventBus() noexcept;

    void Start(std::shared_ptr<IEventProcessor> eventProcessor) noexcept;

    void EnqueueEvent(const BridgeEvent& event) noexcept override;

private:
    QueueHandle_t m_queue = nullptr;
    std::weak_ptr<IEventProcessor> m_eventProcessor;

    static void TaskEntry(void* arg) noexcept;
    void Task() noexcept;
};

#else

class EventBus final : public IEventBus
{
    friend class HomeAssistantBridge;

public:
    EventBus() noexcept {};
    ~EventBus() noexcept {};

    void Start(std::shared_ptr<IEventProcessor> eventProcessor) noexcept {};

    void EnqueueEvent(const BridgeEvent& event) noexcept {};
};

#endif
