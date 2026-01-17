#pragma once

#include "../Module.h"

#include <memory>
#include <vector>

class PushButtonModule final : public Module
{
public:
    PushButtonModule(const Bus& bus, const uint8_t address, const uint8_t numberOfButtons) noexcept;

    static std::unique_ptr<PushButtonModule> CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept;

    ProcessResponse Process() noexcept override;

private:
    const uint8_t m_numberOfButtons;
    bool m_hasPressedButtons = false;

    std::vector<std::shared_ptr<Pin>> m_buttonPins;

    DigitalValue MapButtonState(const uint8_t buttonIndex, const uint16_t data) const noexcept;
};
