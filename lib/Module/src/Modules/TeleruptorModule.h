#pragma once

#include "../ModuleBase.h"

class TeleruptorModule final : public ModuleBase
{
public:
    TeleruptorModule(const Bus& bus, const uint8_t address, const uint16_t initialData);

    ProcessResponse Process() override;

    std::vector<std::weak_ptr<InputPin<DigitalValue>>> GetDigitalInputPins() const override;
    std::vector<std::weak_ptr<OutputPin<DigitalValue>>> GetDigitalOutputPins() const override;

private:
    const uint8_t m_numberOfTeleruptors;
    bool m_feedbackStateInSync = false;
    std::vector<std::shared_ptr<InputPin<DigitalValue>>> m_teleruptorPins;
    std::vector<std::shared_ptr<OutputPin<DigitalValue>>> m_teleruptorFeedbackPins;

    void UpdateTeleruptor(const uint8_t teleruptorIndex, const DigitalValue newValue);
};
