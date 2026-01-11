#include "FilterFactory.h"

#include "Filters/DimmerFilter.h"
#include "Filters/ShutterFilter.h"
#include "Filters/SwitchFilter.h"

std::unique_ptr<Filter> FilterFactory::TryCreateFilterByTypeName(std::string_view typeName)
{
    if (typeName == GetFilterTypeName(FilterType::Switch)) return std::make_unique<SwitchFilter>();
    if (typeName == GetFilterTypeName(FilterType::Dimmer)) return std::make_unique<DimmerFilter>();
    if (typeName == GetFilterTypeName(FilterType::Shutter)) return std::make_unique<ShutterFilter>();
    return nullptr;
}
