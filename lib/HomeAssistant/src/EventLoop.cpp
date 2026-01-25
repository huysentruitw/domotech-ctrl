#ifndef NATIVE_BUILD

#include "EventLoop.h"
#include "Processor.h"

#include "esp_log.h"

#define TAG "HA_LOOP"

EventLoop::EventLoop() noexcept
{
    m_queue = xQueueCreate(20, sizeof(BridgeEvent));
}

EventLoop::~EventLoop() noexcept
{
    BridgeEvent event{};
    event.Type = BridgeEvent::Type::Shutdown;
    xQueueSend(m_queue, &event, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    vQueueDelete(m_queue);
    m_queue = nullptr;
}

void EventLoop::Start(std::shared_ptr<Processor> processor) noexcept
{
    m_processor = processor;
    xTaskCreate(&EventLoop::TaskEntry, TAG, 4096, this, 5, NULL);
}

void EventLoop::EnqueueEvent(const BridgeEvent& event) noexcept
{
    ESP_LOGI(TAG, "EnqueueEvent (Type: %d)", event.Type);
    xQueueSend(m_queue, &event, portMAX_DELAY);
}

void EventLoop::TaskEntry(void* arg) noexcept
{
    static_cast<EventLoop*>(arg)->Task();
}

void EventLoop::Task() noexcept
{
    BridgeEvent event{};

    ESP_LOGI(TAG, "Started");
    while (true)
    {
        xQueueReceive(m_queue, &event, portMAX_DELAY);

        if (auto processor = m_processor.lock())
            processor->Process(event);

        if (event.Type == BridgeEvent::Type::Shutdown)
        {
            ESP_LOGI(TAG, "Shutdown");
            break;
        }
    }
}

void EventLoop::ForwardEvent(void* context, const BridgeEvent& event) noexcept
{
    static_cast<EventLoop*>(context)->EnqueueEvent(event);
}

#endif
