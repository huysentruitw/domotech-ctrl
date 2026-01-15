#pragma once

#include <Filter.h>

#include "IdSanitizer.h"

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

    virtual void ProcessCommand(std::string_view command) const noexcept = 0;
    virtual void SetStateCallback(std::function<void(PinState)> callback) const noexcept = 0;
};

template<class T, class U>
concept Derived = std::is_base_of_v<U, T>;

template<Derived<Filter> TFilter>
class Device : public IDevice
{
public:
    explicit Device(const std::shared_ptr<TFilter>& filter) noexcept
        : m_filter(filter)
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

private:
    std::weak_ptr<TFilter> m_filter;
    std::string m_id;
};
