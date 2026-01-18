#include "PushButtonTemperatureModule.h"

#include <PinFactory.h>

static const uint8_t ButtonMasks6[6] = { 0x10, 0x20, 0x08, 0x04, 0x02, 0x01 };

PushButtonTemperatureModule::PushButtonTemperatureModule(const Bus& bus, const uint8_t address, const uint8_t numberOfButtons) noexcept
    : Module(bus, address, ModuleType::PushButtonWithTemperature)
    , m_numberOfButtons(numberOfButtons)
{
    m_buttonPins.reserve(m_numberOfButtons);
    for (uint8_t i = 0; i < m_numberOfButtons; ++i)
        m_buttonPins.emplace_back(PinFactory::CreateOutputPin<DigitalValue>());

    m_temperaturePin = PinFactory::CreateOutputPin<TemperatureValue>();

    m_outputPins.reserve(m_buttonPins.size() + 1);
    for (const auto& pin : m_buttonPins)
        m_outputPins.emplace_back(pin);

    m_outputPins.emplace_back(m_temperaturePin);
}

std::unique_ptr<PushButtonTemperatureModule> PushButtonTemperatureModule::CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept
{
    return std::make_unique<PushButtonTemperatureModule>(bus, address, (initialData >> 12) & 0x0F);
}

ProcessResponse PushButtonTemperatureModule::Process() noexcept
{
    const bool forceDataExchange = !m_stateInSync || m_hasPressedButtons;
    auto response = Exchange(0x06, forceDataExchange); // CMD6 - Request button state

    if (!response.Success)
        return { .Success = false };

    if (response.RespondedWithTypeAndData)
    {
        for (uint8_t i = 0; i < m_numberOfButtons; ++i)
            m_buttonPins[i]->SetState(MapButtonState(i, response.Data));

        m_hasPressedButtons = (response.Data & 0x0FFF) != 0;

        if (!m_stateInSync || (!m_hasPressedButtons && !forceDataExchange))
        {
            // Request temperature when no buttons are pressed and exchange was initiated by module
            auto temperature = TryRequestTemperature();
            if (temperature != std::nullopt)
                m_temperaturePin->SetState(temperature.value());
        }

        m_stateInSync = true;
        return { .Success = true, .RaisePriority = m_hasPressedButtons };
    }

    if (m_hasPressedButtons)
    {
        m_hasPressedButtons = false;
        for (const auto& pin : m_buttonPins)
            pin->SetState(DigitalValue(false));
    }

    return { .Success = true };
}

std::optional<TemperatureValue> PushButtonTemperatureModule::TryRequestTemperature() noexcept
{
    auto response = Exchange(0x1000, false); // CMD0 - Request status (HSB set to indicate temperature status)
    if (!response.Success || !response.RespondedWithTypeAndData)
        return std::nullopt;

    uint16_t raw = response.Data & 0x01FF; // Mask 9 bits
    uint16_t extended = raw & 0x100 ? raw | 0xFE00 : raw; // Sign-extension for int16_t
    return TemperatureValue(static_cast<int16_t>(extended) * 0.5f);
}

DigitalValue PushButtonTemperatureModule::MapButtonState(const uint8_t buttonIndex, const uint16_t data) const noexcept
{
    if (m_numberOfButtons == 6)
        return DigitalValue((data & ButtonMasks6[buttonIndex]) != 0);

    return DigitalValue(false);
}
