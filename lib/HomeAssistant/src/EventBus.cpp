#ifndef NATIVE_BUILD

#include "EventBus.h"
#include "Processor.h"

#include "esp_log.h"

#define TAG "HA_LOOP"

EventBus::EventBus() noexcept
{
    m_queue = xQueueCreate(20, sizeof(BridgeEvent));
}

EventBus::~EventBus() noexcept
{
    BridgeEvent event{};
    event.Type = BridgeEvent::Type::Shutdown;
    xQueueSend(m_queue, &event, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    vQueueDelete(m_queue);
    m_queue = nullptr;
}

void EventBus::Start(std::shared_ptr<IEventProcessor> eventProcessor) noexcept
{
    m_eventProcessor = eventProcessor;
    xTaskCreate(&EventBus::TaskEntry, TAG, 4096, this, 5, NULL);
}

void EventBus::EnqueueEvent(const BridgeEvent& event) noexcept
{
    ESP_LOGI(TAG, "EnqueueEvent (Type: %d)", event.Type);
    xQueueSend(m_queue, &event, portMAX_DELAY);
}

void EventBus::TaskEntry(void* arg) noexcept
{
    static_cast<EventBus*>(arg)->Task();
}

void EventBus::Task() noexcept
{
    BridgeEvent event{};

    ESP_LOGI(TAG, "Started");
    while (true)
    {
        xQueueReceive(m_queue, &event, portMAX_DELAY);

        if (auto eventProcessor = m_eventProcessor.lock())
            eventProcessor->Process(event);

        if (event.Type == BridgeEvent::Type::Shutdown)
        {
            ESP_LOGI(TAG, "Shutdown");
            break;
        }
    }
}

#endif
