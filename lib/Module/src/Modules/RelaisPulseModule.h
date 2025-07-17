#pragma once

#include "../ModuleBase.h"

class RelaisPulseModule final : public ModuleBase
{
public:
    RelaisPulseModule(const uint8_t address, const uint16_t initialData);

    bool Process(const BusProtocol& bus) override;

    std::vector<std::weak_ptr<InputPin<bool>>> GetDigitalInputPins() const override;
    std::vector<std::weak_ptr<OutputPin<bool>>> GetDigitalOutputPins() const override;

private:
    const uint8_t numberOfRelais;
    std::vector<std::shared_ptr<InputPin<bool>>> relaisPins;
    std::vector<std::shared_ptr<OutputPin<bool>>> relaisFeedbackPins;
};
