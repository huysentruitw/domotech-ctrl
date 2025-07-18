#pragma once

#include "../ModuleBase.h"

class DimmerModule final : public ModuleBase
{
public:
    DimmerModule(const Bus& bus, const uint8_t address, const uint16_t initialData);

    ProcessResponse Process() override;

    std::vector<std::weak_ptr<InputPin<DimmerControlValue>>> GetDimmerControlPins() const override;

    std::string ToString() const override;
    static std::unique_ptr<DimmerModule> TryConstructFromString(const Bus& bus, const std::string& encodedModuleInfo);

private:
    const uint8_t m_numberOfChannels;
    std::vector<std::shared_ptr<InputPin<DimmerControlValue>>> m_dimmerControlPins;

    void UpdateChannel(const uint8_t channelIndex, const DimmerControlValue newValue);
};
