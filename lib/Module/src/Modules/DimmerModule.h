#pragma once

#include <IPinObserver.h>

#include "../Module.h"

#include <memory>
#include <vector>

class DimmerModule final : public Module, private IPinObserver
{
public:
    DimmerModule(const Bus& bus, const uint8_t address, const uint8_t numberOfChannels) noexcept;

    static std::unique_ptr<DimmerModule> CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept;

    ProcessResponse Process() noexcept override;

private:
    const uint8_t m_numberOfChannels;
    std::vector<std::shared_ptr<Pin>> m_dimmerControlPins;

    void OnPinStateChanged(const Pin& pin) noexcept override;
    void UpdateChannel(const uint8_t channelIndex, const DimmerControlValue newValue) noexcept;
};
