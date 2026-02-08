#include "PushButtonModule.h"

#include <PinFactory.h>

static const uint8_t ButtonMasks8[8] = { 0x10, 0x20, 0x40, 0x80, 0x08, 0x04, 0x02, 0x01 };

PushButtonModule::PushButtonModule(const Bus& bus, const uint8_t address, const uint8_t numberOfButtons) noexcept
    : Module(bus, address, ModuleType::PushButton)
    , m_numberOfButtons(numberOfButtons)
{
    m_buttonPins.reserve(m_numberOfButtons);
    for (uint8_t i = 0; i < m_numberOfButtons; ++i)
        m_buttonPins.emplace_back(PinFactory::CreateOutputPin<DigitalValue>());

    m_outputPins.reserve(m_buttonPins.size());
    for (const auto& pin : m_buttonPins)
        m_outputPins.emplace_back(pin);
}

std::unique_ptr<PushButtonModule> PushButtonModule::CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept
{
    return std::make_unique<PushButtonModule>(bus, address, (initialData >> 12) & 0x0F);
}

uint16_t PushButtonModule::GenerateInitialData() const noexcept
{
    return static_cast<uint16_t>(m_numberOfButtons) << 12;
}

ProcessResponse PushButtonModule::Process() noexcept
{
    const auto forceDataExchange = m_hasPressedButtons;
    auto response = Exchange(0x06, forceDataExchange); // CMD6 - Request button state

    if (!response.Success)
        return { .Success = false };

    if (response.RespondedWithTypeAndData)
    {
        for (uint8_t i = 0; i < m_numberOfButtons; ++i)
            m_buttonPins[i]->SetState(MapButtonState(i, response.Data));

        m_hasPressedButtons = (response.Data & 0x0FFF) != 0;
        return { .Success = true, .BoostPriority = m_hasPressedButtons };
    }

    if (m_hasPressedButtons)
    {
        m_hasPressedButtons = false;
        for (const auto& pin : m_buttonPins)
            pin->SetState(DigitalValue(false));
    }

    return { .Success = true };
}

DigitalValue PushButtonModule::MapButtonState(const uint8_t buttonIndex, const uint16_t data) const noexcept
{
    if (m_numberOfButtons == 8)
        return DigitalValue((data & ButtonMasks8[buttonIndex]) != 0);

    return DigitalValue(false);
}
