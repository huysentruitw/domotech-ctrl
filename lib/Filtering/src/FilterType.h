#pragma once

#include <optional>
#include <string_view>

enum class FilterType
{
    Switch = 1,
    Light = 2,
    Dimmer = 3,
    Shutter = 4,
    Climate = 5,
    DigitalPassthrough = 6,
};

std::string_view GetFilterTypeName(FilterType filterType) noexcept;
std::optional<FilterType> GetFilterType(std::string_view filterTypeName) noexcept;
