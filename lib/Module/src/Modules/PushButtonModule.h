#pragma once

#include "../ModuleBase.h"

class PushButtonModule final : public ModuleBase
{
public:
    PushButtonModule(const Bus& bus, const uint8_t address, const uint16_t initialData);

    ProcessResponse Process() override;

    std::vector<std::weak_ptr<OutputPin<bool>>> GetDigitalOutputPins() const override;

private:
    const uint8_t m_numberOfButtons;
    std::vector<std::shared_ptr<OutputPin<bool>>> m_buttonPins;
    bool m_hasPressedButtons = false;

    bool MapButtonState(const uint8_t buttonIndex, const uint16_t data) const;
};
