#include "FilterFactory.h"

#include "Filters/DimmerFilter.h"
#include "Filters/ShutterFilter.h"
#include "Filters/ToggleFilter.h"

std::unique_ptr<Filter> FilterFactory::TryCreateFilterByTypeName(std::string_view typeName)
{
    if (typeName == GetFilterTypeName(FilterType::Dimmer)) return std::make_unique<DimmerFilter>();
    if (typeName == GetFilterTypeName(FilterType::Shutter)) return std::make_unique<ShutterFilter>();
    if (typeName == GetFilterTypeName(FilterType::Toggle)) return std::make_unique<ToggleFilter>();
    return nullptr;
}
