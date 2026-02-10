#include "FilterFactory.h"

#include "Filters/ClimateFilter.h"
#include "Filters/DigitalPassthroughFilter.h"
#include "Filters/DimmerFilter.h"
#include "Filters/LightFilter.h"
#include "Filters/ShutterFilter.h"
#include "Filters/SwitchFilter.h"

std::unique_ptr<Filter> FilterFactory::TryCreateFilterByTypeName(std::string_view typeName, std::string_view id) noexcept
{
    if (typeName == GetFilterTypeName(FilterType::Switch)) return std::make_unique<SwitchFilter>(id);
    if (typeName == GetFilterTypeName(FilterType::Light)) return std::make_unique<LightFilter>(id);
    if (typeName == GetFilterTypeName(FilterType::Dimmer)) return std::make_unique<DimmerFilter>(id);
    if (typeName == GetFilterTypeName(FilterType::Shutter)) return std::make_unique<ShutterFilter>(id);
    if (typeName == GetFilterTypeName(FilterType::Climate)) return std::make_unique<ClimateFilter>(id);
    if (typeName == GetFilterTypeName(FilterType::DigitalPassthrough)) return std::make_unique<DigitalPassthroughFilter>(id);
    return nullptr;
}

std::string FilterFactory::GetKnownFiltersIni() noexcept
{
    IniWriter iniWriter;

    SwitchFilter().WriteDescriptor(iniWriter);
    LightFilter().WriteDescriptor(iniWriter);
    DimmerFilter().WriteDescriptor(iniWriter);
    ShutterFilter().WriteDescriptor(iniWriter);
    ClimateFilter().WriteDescriptor(iniWriter);
    DigitalPassthroughFilter().WriteDescriptor(iniWriter);

    return iniWriter.GetContent();
}
