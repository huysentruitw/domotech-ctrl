#pragma once

#include <string>
#include <unordered_map>

enum class FilterType
{
    Toggle = 1,
    Shutter = 2,
    Dimmer = 3,
};

const std::string GetFilterTypeName(FilterType filterType);
