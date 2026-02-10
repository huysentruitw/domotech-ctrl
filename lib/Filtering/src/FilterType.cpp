#include "FilterType.h"

const auto& FilterIdentifiers()
{
    using enum FilterType;
    static constexpr std::pair<FilterType, std::string_view> identifiers[] =
    {
        { FilterType::Switch, "Switch" },
        { FilterType::Light, "Light" },
        { FilterType::Dimmer, "Dimmer" },
        { FilterType::Shutter, "Shutter" },
        { FilterType::Climate, "Climate" },
        { FilterType::DigitalPassthrough, "DigitalPassthrough" },
    };
    return identifiers;
}

std::string_view GetFilterTypeName(FilterType filterType) noexcept
{
    for (auto&& [key, value] : FilterIdentifiers())
    {
        if (key == filterType)
            return value;
    }

    return "Unknown";
}

std::optional<FilterType> GetFilterType(std::string_view filterTypeName) noexcept
{
    for (auto&& [key, value] : FilterIdentifiers())
    {
        if (value == filterTypeName)
            return key;
    }

    return std::nullopt;
}
