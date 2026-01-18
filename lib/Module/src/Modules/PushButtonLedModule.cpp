#include "PushButtonLedModule.h"

#include <PinFactory.h>

static const uint8_t ButtonMasks4[8] = { 0x08, 0x04, 0x02, 0x01 };

PushButtonLedModule::PushButtonLedModule(const Bus& bus, const uint8_t address, const uint8_t numberOfButtons) noexcept
    : Module(bus, address, ModuleType::PushButtonWithLed)
    , m_numberOfButtons(numberOfButtons)
{
    m_buttonPins.reserve(m_numberOfButtons);
    m_ledPins.reserve(m_numberOfButtons);
    for (uint8_t i = 0; i < m_numberOfButtons; ++i)
    {
        m_buttonPins.emplace_back(PinFactory::CreateOutputPin<DigitalValue>());

        const auto onStateChange = [this, i](const Pin& pin)
        {
            UpdateLed(i, pin.GetStateAs<DigitalValue>());
        };
        m_ledPins.emplace_back(PinFactory::CreateInputPin<DigitalValue>(onStateChange));
    }

    m_outputPins.reserve(m_buttonPins.size());
    for (const auto& pin : m_buttonPins)
        m_outputPins.emplace_back(pin);

    m_inputPins.reserve(m_ledPins.size());
    for (const auto& pin : m_ledPins)
        m_inputPins.emplace_back(pin);
}

std::unique_ptr<PushButtonLedModule> PushButtonLedModule::CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept
{
    return std::make_unique<PushButtonLedModule>(bus, address, (initialData >> 12) & 0x0F);
}

ProcessResponse PushButtonLedModule::Process() noexcept
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

void PushButtonLedModule::UpdateLed(const uint8_t ledIndex, const DigitalValue newValue) noexcept
{
    uint16_t command = newValue ? 0x01 : 0x02; // CMD1 - Set LED ON, CMD2 - Set LED OFF
    Exchange(command | (0x80 >> ledIndex), true);
}

DigitalValue PushButtonLedModule::MapButtonState(const uint8_t buttonIndex, const uint16_t data) const noexcept
{
    if (m_numberOfButtons == 4)
        return DigitalValue((data & ButtonMasks4[buttonIndex]) != 0);

    return DigitalValue(false);
}
