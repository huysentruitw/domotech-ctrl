#pragma once

#include <Filter.h>

#include <memory>

class IBridge
{
public:
    virtual ~IBridge() noexcept = default;

    virtual bool RegisterAsDevice(std::weak_ptr<Filter> filter) noexcept = 0;
    virtual bool UnregisterAsDevice(std::weak_ptr<Filter> filter) noexcept = 0;
};
