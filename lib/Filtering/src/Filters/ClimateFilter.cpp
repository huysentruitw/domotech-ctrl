#include "ClimateFilter.h"

#include <PinFactory.h>

ClimateFilter::ClimateFilter(std::string_view id) noexcept
    : Filter(FilterType::Climate, id)
{
}

void ClimateFilter::SetState(DigitalValue state) noexcept
{
}

void ClimateFilter::OnPinStateChanged(const Pin& pin) noexcept
{
}
