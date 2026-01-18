#include "InputModule.h"

#include <PinFactory.h>

static const uint8_t InputMasks8[8] = { 0x10, 0x20, 0x40, 0x80, 0x08, 0x04, 0x02, 0x01 };

InputModule::InputModule(const Bus& bus, const uint8_t address, const uint8_t numberOfInputs) noexcept
    : Module(bus, address, ModuleType::Input)
    , m_numberOfInputs(numberOfInputs)
{
    m_pins.reserve(m_numberOfInputs);
    for (uint8_t i = 0; i < m_numberOfInputs; ++i)
        m_pins.emplace_back(PinFactory::CreateOutputPin<DigitalValue>());
    
    m_outputPins.reserve(m_pins.size());
    for (const auto& pin : m_pins)
        m_outputPins.emplace_back(pin);
}

std::unique_ptr<InputModule> InputModule::CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept
{
    return std::make_unique<InputModule>(bus, address, (initialData >> 12) & 0x0F);
}

ProcessResponse InputModule::Process() noexcept
{
    const bool forceDataExchange = !m_stateInSync;
    auto response = Exchange(0x06, forceDataExchange); // CMD6 - Request feedback state

    if (!response.Success) {
        return { .Success = false };
    }

    if (response.RespondedWithTypeAndData)
    {
        for (uint8_t i = 0; i < m_numberOfInputs; ++i)
            m_pins[i]->SetState(MapInputState(i, response.Data));

        m_stateInSync = true;
    }

    return { .Success = true };
}

DigitalValue InputModule::MapInputState(const uint8_t inputIndex, const uint16_t data) const noexcept
{
    if (m_numberOfInputs == 8)
        return DigitalValue((data & InputMasks8[inputIndex]) != 0);

    return DigitalValue(false);
}
