#include "FilterFactory.h"

#include "Filters/DimmerFilter.h"
#include "Filters/ShutterFilter.h"
#include "Filters/ToggleFilter.h"

std::unique_ptr<Filter> FilterFactory::TryCreateFilterByTypeName(const std::string filterName)
{
    if (filterName == GetFilterTypeName(FilterType::Dimmer)) return std::make_unique<DimmerFilter>();
    if (filterName == GetFilterTypeName(FilterType::Shutter)) return std::make_unique<ShutterFilter>();
    if (filterName == GetFilterTypeName(FilterType::Toggle)) return std::make_unique<ToggleFilter>();
    return nullptr;
}
