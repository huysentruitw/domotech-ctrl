#include "FilterType.h"

const std::string GetFilterTypeName(FilterType filterType)
{
    static const std::unordered_map<FilterType, std::string> identifiers = {
        { FilterType::Toggle, "Toggle" },
        { FilterType::Shutter, "Shutter" },
        { FilterType::Dimmer, "Dimmer" },
    };

    return identifiers.contains(filterType)
        ? identifiers.at(filterType)
        : "Unknown";
}
