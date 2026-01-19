#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

class ClimateFilter final : public Filter
{
public:
    ClimateFilter(std::string_view id = {}) noexcept;

    void SetState(DigitalValue state) noexcept;
    bool SetStateChangedCallback(const std::function<void(const ClimateFilter&, DigitalValue)>& callback) noexcept;

private:
};
