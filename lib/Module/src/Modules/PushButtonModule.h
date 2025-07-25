#pragma once

#include "../ModuleBase.h"

#include <memory>
#include <vector>

class PushButtonModule final : public ModuleBase
{
public:
    PushButtonModule(const Bus& bus, const uint8_t address, const uint8_t numberOfButtons);

    static std::unique_ptr<PushButtonModule> CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData);

    ProcessResponse Process() override;

    std::vector<std::weak_ptr<Pin>> GetOutputPins() const override;

private:
    const uint8_t m_numberOfButtons;
    std::vector<std::shared_ptr<Pin>> m_buttonPins;
    bool m_hasPressedButtons = false;

    DigitalValue MapButtonState(const uint8_t buttonIndex, const uint16_t data) const;
};
