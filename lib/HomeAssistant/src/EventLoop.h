#pragma once

#include "BridgeEvent.h"
#include "IEventBus.h"

class Processor; // forward declaration

#ifndef NATIVE_BUILD

#include "freertos/FreeRTOS.h"

class EventLoop final : public IEventBus
{
    friend class HomeAssistantBridge;

public:
    EventLoop() noexcept;
    ~EventLoop() noexcept;

    void Start(std::shared_ptr<Processor> processor) noexcept;

    void EnqueueEvent(const BridgeEvent& event) noexcept;

private:
    QueueHandle_t m_queue = nullptr;
    std::weak_ptr<Processor> m_processor;

    static void TaskEntry(void* arg) noexcept;
    void Task() noexcept;

    static void ForwardEvent(void* context, const BridgeEvent& event) noexcept;
};

#else

class EventLoop final : public IEventBus
{
    friend class HomeAssistantBridge;

public:
    EventLoop() noexcept {};
    ~EventLoop() noexcept {};

    void Start(std::shared_ptr<Processor> processor) noexcept {};

    void EnqueueEvent(const BridgeEvent& event) noexcept {};

private:
    static void ForwardEvent(void* context, const BridgeEvent& event) noexcept {};
};

#endif
