#pragma once

#include <Filter.h>

#include "IdSanitizer.h"
#include "IEventBus.h"

#include <memory>
#include <string>
#include <type_traits>

class IDevice
{
public:
    virtual ~IDevice() noexcept = default;
    virtual std::string_view GetId() const noexcept = 0;

    virtual size_t BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept = 0;
    virtual size_t BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept = 0;

    virtual void SubscribeToStateChanges() noexcept = 0;
    virtual void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept = 0;
    virtual void EnqueueCurrentState() noexcept = 0;
};

template<class T, class U>
concept Derived = std::is_base_of_v<U, T>;

template<Derived<Filter> TFilter>
class Device : public IDevice
{
public:
    explicit Device(const std::shared_ptr<TFilter>& filter, const std::weak_ptr<IEventBus>& eventPublisher) noexcept
        : m_filter(filter)
        , m_eventPublisher(eventPublisher)
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
        return m_eventPublisher.lock();
    }

private:
    const std::weak_ptr<TFilter> m_filter;
    const std::weak_ptr<IEventBus> m_eventPublisher;
    const std::string m_id;
};
