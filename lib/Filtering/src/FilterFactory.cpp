#include "FilterFactory.h"

#include "Filters/DimmerFilter.h"
#include "Filters/LightFilter.h"
#include "Filters/ShutterFilter.h"
#include "Filters/SwitchFilter.h"

std::unique_ptr<Filter> FilterFactory::TryCreateFilterByTypeName(std::string_view typeName, std::string_view id)
{
    if (typeName == GetFilterTypeName(FilterType::Switch)) return std::make_unique<SwitchFilter>(id);
    if (typeName == GetFilterTypeName(FilterType::Light)) return std::make_unique<LightFilter>(id);
    if (typeName == GetFilterTypeName(FilterType::Dimmer)) return std::make_unique<DimmerFilter>(id);
    if (typeName == GetFilterTypeName(FilterType::Shutter)) return std::make_unique<ShutterFilter>(id);
    return nullptr;
}
