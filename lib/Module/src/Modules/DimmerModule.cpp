#include "DimmerModule.h"

#include <PinFactory.h>

DimmerModule::DimmerModule(const Bus& bus, const uint8_t address, const uint8_t numberOfChannels)
    : Module(bus, address, ModuleType::Dimmer)
    , m_numberOfChannels(numberOfChannels)
{
    m_dimmerControlPins.reserve(m_numberOfChannels);
    for (uint8_t i = 0; i < m_numberOfChannels; ++i) {
        const auto onStateChange = [this, i](const Pin& pin)
        {
            UpdateChannel(i, pin.GetStateAs<DimmerControlValue>());
        };

        m_dimmerControlPins.emplace_back(PinFactory::CreateInputPin<DimmerControlValue>(onStateChange));
    }

    m_inputPins.reserve(m_dimmerControlPins.size());
    for (const auto& pin : m_dimmerControlPins) {
        m_inputPins.emplace_back(pin);
    }
}

std::unique_ptr<DimmerModule> DimmerModule::CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData)
{
    return std::make_unique<DimmerModule>(bus, address, 16);
}

ProcessResponse DimmerModule::Process()
{
    auto response = Poll();
    return { .Success = response.Success };
}

void DimmerModule::UpdateChannel(const uint8_t channelIndex, const DimmerControlValue newValue)
{
    uint16_t command = 0x02; // CMD2 - Set dimmer time
    Exchange(command | (newValue.GetFadeTimeInSeconds() << 8));

    command = 0x01; // CMD1 - Set dimmer percentage
    Exchange(command | (newValue.GetPercentage() << 8) | ((channelIndex + 1) << 4));
}
