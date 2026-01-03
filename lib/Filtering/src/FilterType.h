#pragma once

#include <string>

enum class FilterType
{
    Toggle = 1,
    Shutter = 2,
    Dimmer = 3,
};

const std::string_view GetFilterTypeName(FilterType filterType);
