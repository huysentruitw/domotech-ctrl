#pragma once

#include <IPinObserver.h>

#include "../Module.h"

#include <memory>
#include <vector>

class PushButtonLedModule final : public Module, private IPinObserver
{
public:
    PushButtonLedModule(const Bus& bus, const uint8_t address, const uint8_t numberOfButtons) noexcept;

    static std::unique_ptr<PushButtonLedModule> CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept;

    ProcessResponse Process() noexcept override;

private:
    const uint8_t m_numberOfButtons;
    bool m_hasPressedButtons = false;

    std::vector<std::shared_ptr<Pin>> m_buttonPins;
    std::vector<std::shared_ptr<Pin>> m_ledPins;

    void OnPinStateChanged(const Pin& pin) noexcept override;
    void UpdateLed(const uint8_t ledIndex, const DigitalValue newValue) noexcept;
    DigitalValue MapButtonState(const uint8_t buttonIndex, const uint16_t data) const noexcept;
};
