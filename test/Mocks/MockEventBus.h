#pragma once

#include <IEventBus.h>
#include <vector>

class MockEventBus : public IEventBus
{
public:
    MockEventBus() noexcept
        : m_events()
    {
    }

    void EnqueueEvent(const BridgeEvent& event) noexcept override
    {
        m_events.push_back(event);
    }

    // Helper methods for testing
    size_t GetEventCount() const noexcept
    {
        return m_events.size();
    }

    // Check if any event matches the type field
    bool HasEventWithTypeValue(int typeValue) const noexcept
    {
        for (const auto& event : m_events)
        {
            // Compare the Type field value directly
            if (static_cast<int>(event.Type) == typeValue)
                return true;
        }
        return false;
    }

    const BridgeEvent& GetLastEvent() const noexcept
    {
        return m_events.back();
    }

    const BridgeEvent& GetEventAt(size_t index) const noexcept
    {
        return m_events[index];
    }

    void ClearEvents() noexcept
    {
        m_events.clear();
    }

private:
    std::vector<BridgeEvent> m_events;
};
