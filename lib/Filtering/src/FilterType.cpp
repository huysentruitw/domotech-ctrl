#include "FilterType.h"

const std::string_view GetFilterTypeName(FilterType filterType)
{
    using enum FilterType;

    constexpr std::pair<FilterType, std::string_view> identifiers[] = {
        { FilterType::Toggle, "Toggle" },
        { FilterType::Shutter, "Shutter" },
        { FilterType::Dimmer, "Dimmer" },
    };

    for (auto&& [key, value] : identifiers) {
        if (key == filterType)
            return value;
    }

    return "Unknown";
}
