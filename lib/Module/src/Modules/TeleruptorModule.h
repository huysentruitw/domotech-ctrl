#pragma once

#include <IPinObserver.h>

#include "../Module.h"

#include <memory>
#include <vector>

class TeleruptorModule final : public Module, private IPinObserver
{
public:
    TeleruptorModule(const Bus& bus, const uint8_t address, const uint8_t numberOfTeleruptors) noexcept;

    static std::unique_ptr<TeleruptorModule> CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept;

    ProcessResponse Process() noexcept override;

private:
    const uint8_t m_numberOfTeleruptors;
    bool m_feedbackStateInSync = false;
    std::vector<std::shared_ptr<Pin>> m_teleruptorPins;
    std::vector<std::shared_ptr<Pin>> m_teleruptorFeedbackPins;

    void OnPinStateChanged(const Pin& pin) noexcept override;
    void UpdateTeleruptor(const uint8_t teleruptorIndex, const DigitalValue newValue) noexcept;
};
