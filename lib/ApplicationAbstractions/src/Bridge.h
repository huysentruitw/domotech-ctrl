#pragma once

#include <Filter.h>

#include <memory>

class Bridge
{
public:
    virtual ~Bridge() = default;

    virtual void RegisterFilter(std::weak_ptr<Filter> filter) = 0;
};
