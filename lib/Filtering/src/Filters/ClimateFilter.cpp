#include "ClimateFilter.h"

#include <PinFactory.h>

ClimateFilter::ClimateFilter(std::string_view id) noexcept
    : Filter(FilterType::Climate, id)
{
}

void ClimateFilter::SetState(DigitalValue state) noexcept
{
}

bool ClimateFilter::SetStateCallback(const std::function<void(const ClimateFilter&, DigitalValue)>& callback) noexcept
{
    return false;
}
