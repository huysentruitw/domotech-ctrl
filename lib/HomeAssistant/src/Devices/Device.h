#pragma once

#include <Filter.h>
#include <StaticList.h>

#include "IdSanitizer.h"
#include "IEventBus.h"

#include <memory>
#include <string>
#include <type_traits>

struct StateMessage; // Forward declaration
using StateMessageList = StaticList<StateMessage, 2>;

class IDevice
{
public:
    virtual ~IDevice() noexcept = default;
    virtual std::string_view GetId() const noexcept = 0;

    virtual bool BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept = 0;
    virtual bool BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept = 0;
    virtual bool BuildStateMessages(StateMessageList& out) const noexcept = 0;

    virtual void SubscribeToStateChanges() noexcept = 0;
    virtual void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept = 0;
};

struct StateMessage
{
    char Topic[64];
    char Payload[64];
    bool Retain;
};

template<class T, class U>
concept Derived = std::is_base_of_v<U, T>;

template<Derived<Filter> TFilter>
class Device : public IDevice
{
public:
    explicit Device(const std::shared_ptr<TFilter>& filter, const std::weak_ptr<IEventBus>& eventBus) noexcept
        : m_filter(filter)
        , m_eventBus(eventBus)
        , m_id(IdSanitizer::Sanitize(filter->GetId())) {}

    std::string_view GetId() const noexcept override
    {
        return m_id;
    }

protected:
    std::shared_ptr<TFilter> TryGetFilter() const noexcept
    {
        return m_filter.lock();
    }

    std::shared_ptr<IEventBus> TryGetEventBus() const noexcept
    {
        return m_eventBus.lock();
    }

private:
    const std::weak_ptr<TFilter> m_filter;
    const std::weak_ptr<IEventBus> m_eventBus;
    const std::string m_id;
};
