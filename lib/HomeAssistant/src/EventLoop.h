#pragma once

#include "BridgeEvent.h"

#ifndef NATIVE_BUILD

#include "freertos/FreeRTOS.h"

class Processor; // forward declaration

class EventLoop final
{
    friend class HomeAssistantBridge;

public:
    EventLoop(Processor& processor) noexcept;
    ~EventLoop() noexcept;

    void Start() noexcept;

    void EnqueueEvent(const BridgeEvent& event) noexcept;

private:
    Processor& m_processor;
    QueueHandle_t m_queue = nullptr;

    static void TaskEntry(void* arg) noexcept;
    void Task() noexcept;

    static void ForwardEvent(void* context, const BridgeEvent& event) noexcept;
};

#else

class EventLoop final
{
public:
    EventLoop(Processor& processor) {};
};

#endif
