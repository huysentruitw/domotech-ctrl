#pragma once

#include <string>

enum class FilterType
{
    Switch = 1,
    Dimmer = 2,
    Shutter = 3,
};

std::string_view GetFilterTypeName(FilterType filterType);
