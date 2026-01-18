#include "FilterType.h"

std::string_view GetFilterTypeName(FilterType filterType) noexcept
{
    using enum FilterType;

    constexpr std::pair<FilterType, std::string_view> identifiers[] =
    {
        { FilterType::Switch, "Switch" },
        { FilterType::Light, "Light" },
        { FilterType::Dimmer, "Dimmer" },
        { FilterType::Shutter, "Shutter" },
        { FilterType::Climate, "Climate" },
        { FilterType::DigitalPassthrough, "DigitalPassthrough" },
    };

    for (auto&& [key, value] : identifiers)
    {
        if (key == filterType)
            return value;
    }

    return "Unknown";
}
