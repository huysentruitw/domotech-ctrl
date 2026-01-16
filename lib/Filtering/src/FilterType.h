#pragma once

#include <string>

enum class FilterType
{
    Switch = 1,
    Light = 2,
    Dimmer = 3,
    Shutter = 4,
    Climate = 5,
};

std::string_view GetFilterTypeName(FilterType filterType) noexcept;
