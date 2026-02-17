#pragma once

#include <Filter.h>
#include <PinFactory.h>

class MockFilter : public Filter
{
public:
    MockFilter(std::string_view id)
        : Filter(FilterType::Switch, id)
    {
    }
};
