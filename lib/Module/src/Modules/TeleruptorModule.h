#pragma once

#include "../ModuleBase.h"

#include <memory>
#include <vector>

class TeleruptorModule final : public ModuleBase
{
public:
    TeleruptorModule(const Bus& bus, const uint8_t address, const uint8_t numberOfTeleruptors);

    static std::unique_ptr<TeleruptorModule> CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData);

    ProcessResponse Process() override;

    std::vector<std::weak_ptr<Pin>> GetInputPins() const override;
    std::vector<std::weak_ptr<Pin>> GetOutputPins() const override;

private:
    const uint8_t m_numberOfTeleruptors;
    bool m_feedbackStateInSync = false;
    std::vector<std::shared_ptr<Pin>> m_teleruptorPins;
    std::vector<std::shared_ptr<Pin>> m_teleruptorFeedbackPins;

    void UpdateTeleruptor(const uint8_t teleruptorIndex, const DigitalValue newValue);
};
