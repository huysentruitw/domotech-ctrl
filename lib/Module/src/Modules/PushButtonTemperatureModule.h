#pragma once

#include "../Module.h"

#include <memory>
#include <optional>
#include <vector>

class PushButtonTemperatureModule final : public Module
{
public:
    PushButtonTemperatureModule(const Bus& bus, const uint8_t address, const uint8_t numberOfButtons) noexcept;

    static std::unique_ptr<PushButtonTemperatureModule> CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept;

    ProcessResponse Process() noexcept override;

private:
    const uint8_t m_numberOfButtons;
    bool m_stateInSync = false;
    bool m_hasPressedButtons = false;

    std::vector<std::shared_ptr<Pin>> m_buttonPins;
    std::shared_ptr<Pin> m_temperaturePin;

    std::optional<TemperatureValue> TryRequestTemperature() noexcept;
    DigitalValue MapButtonState(const uint8_t buttonIndex, const uint16_t data) const noexcept;
};
