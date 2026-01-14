#pragma once

#include <Filter.h>

#include <memory>

class Bridge
{
public:
    virtual ~Bridge() noexcept = default;

    virtual void RegisterFilter(std::weak_ptr<Filter> filter) noexcept = 0;
    virtual void UnregisterFilter(std::weak_ptr<Filter> filter) noexcept = 0;
};
