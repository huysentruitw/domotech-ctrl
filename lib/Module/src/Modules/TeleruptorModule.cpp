#include "TeleruptorModule.h"

#include <PinFactory.h>

TeleruptorModule::TeleruptorModule(const Bus& bus, const uint8_t address, const uint8_t numberOfTeleruptors)
    : Module(bus, address, ModuleType::Teleruptor)
    , m_numberOfTeleruptors(numberOfTeleruptors)
{
    m_teleruptorPins.reserve(m_numberOfTeleruptors);
    m_teleruptorFeedbackPins.reserve(m_numberOfTeleruptors);
    for (uint8_t i = 0; i < m_numberOfTeleruptors; ++i) {
        const auto onStateChange = [this, i](const Pin& pin)
        {
            UpdateTeleruptor(i, pin.GetStateAs<DigitalValue>());
        };

        m_teleruptorPins.emplace_back(PinFactory::CreateInputPin<DigitalValue>(onStateChange));
        m_teleruptorFeedbackPins.emplace_back(PinFactory::CreateOutputPin<DigitalValue>());
    }

    m_inputPins.reserve(m_teleruptorPins.size());
    for (const auto& pin : m_teleruptorPins) {
        m_inputPins.emplace_back(pin);
    }

    m_outputPins.reserve(m_teleruptorFeedbackPins.size());
    for (const auto& pin : m_teleruptorFeedbackPins) {
        m_outputPins.emplace_back(pin);
    }
}

std::unique_ptr<TeleruptorModule> TeleruptorModule::CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData)
{
    return std::make_unique<TeleruptorModule>(bus, address, initialData & 0x0F);
}

ProcessResponse TeleruptorModule::Process()
{
    auto response = Poll();

    if (!response.Success) {
        return { .Success = false };
    }

    // The module response with type and data when one or more relais feedback pins are changed
    // The response doesn't include the current state, so we need to request it separately
    if (response.RespondedWithTypeAndData || !m_feedbackStateInSync) {
        response = Exchange(0x06); // CMD6 - Request feedback state

        if (!response.Success || !response.RespondedWithTypeAndData) {
            return { .Success = false };
        }

        for (uint8_t i = 0; i < m_numberOfTeleruptors; ++i) {
            m_teleruptorFeedbackPins[i]->SetState(DigitalValue((response.Data & (1 << i)) != 0));
        }

        m_feedbackStateInSync = true;
    }

    return { .Success = true };
}

void TeleruptorModule::UpdateTeleruptor(const uint8_t teleruptorIndex, const DigitalValue newValue)
{
    uint16_t command = newValue ? 0x01 : 0x02; // CMD1 - Set teleruptor ON, CMD2 - Set teleruptor OFF
    Exchange(command | (teleruptorIndex << 4));
}
