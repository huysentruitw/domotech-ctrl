#pragma once

#include <IPinObserver.h>

#include "Filter.h"

#include <memory>
#include <vector>

class ClimateFilter final : public Filter, private IPinObserver
{
public:
    ClimateFilter(std::string_view id = {}) noexcept;

    void SetState(DigitalValue state) noexcept;

private:
    void OnPinStateChanged(const Pin& pin) noexcept override;
};
