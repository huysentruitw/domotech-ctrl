#pragma once

#include "../ModuleBase.h"

class PushButtonModule final : public ModuleBase
{
public:
    PushButtonModule(const uint8_t address, const uint16_t initialData);

    bool Process(const BusProtocol& bus) override;

    std::vector<std::weak_ptr<OutputPin<bool>>> GetDigitalOutputPins() const override;

private:
    const uint8_t numberOfButtons;
    std::vector<std::shared_ptr<OutputPin<bool>>> buttonPins;
    bool hasPressedButtons = false;

    bool MapButtonState(const uint8_t buttonIndex, const uint16_t data) const;
};
