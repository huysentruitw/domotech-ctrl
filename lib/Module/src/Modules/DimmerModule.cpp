#include <sstream>
#include <string>

#include <KnownModuleIdentifiers.h>

#include "DimmerModule.h"

DimmerModule::DimmerModule(const Bus& bus, const uint8_t address, const uint16_t initialData)
    : ModuleBase(bus, address, ModuleType::Dimmer)
    , m_numberOfChannels(12)
{
    for (uint8_t i = 0; i < m_numberOfChannels; ++i) {
        m_dimmerControlPins.push_back(std::make_shared<InputPin<DimmerControlValue>>([this, i](DimmerControlValue value) { UpdateChannel(i, value); }, DimmerControlValue(0, 0)));
    }
}

ProcessResponse DimmerModule::Process()
{
    auto response = Poll();
    return { .Success = response.Success };
}

std::vector<std::weak_ptr<InputPin<DimmerControlValue>>> DimmerModule::GetDimmerControlPins() const
{
    std::vector<std::weak_ptr<InputPin<DimmerControlValue>>> inputPins;
    for (const auto& pin : m_dimmerControlPins) {
        inputPins.push_back(pin);
    }

    return inputPins;
}

void DimmerModule::UpdateChannel(const uint8_t channelIndex, const DimmerControlValue newValue)
{
    uint16_t command = 0x02; // CMD2 - Set dimmer time
    Exchange(command | (newValue.GetFadeTimeInSeconds() << 8));

    command = 0x01; // CMD1 - Set dimmer percentage
    Exchange(command | (newValue.GetPercentage() << 8) | ((channelIndex + 1) << 4));
}

std::string DimmerModule::ToString() const
{
    return (std::string)KnownModuleIdentifiers::Dimmer + " " + std::to_string(GetAddress()) + " " + std::to_string(m_numberOfChannels);
}

std::unique_ptr<DimmerModule> DimmerModule::TryConstructFromString(const Bus& bus, const std::string& encodedModuleInfo)
{
    std::istringstream stream(encodedModuleInfo);
    std::string identifier;
    uint8_t address;
    uint8_t numberOfChannels;

    if (!(stream >> identifier) || identifier != KnownModuleIdentifiers::Dimmer ||
        !(stream >> address) || !(stream >> numberOfChannels)) {
        return nullptr; // Invalid format
    }

    return std::make_unique<DimmerModule>(bus, address, 0);
}
