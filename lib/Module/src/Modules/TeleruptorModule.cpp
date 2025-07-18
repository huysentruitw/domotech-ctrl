#include "TeleruptorModule.h"

TeleruptorModule::TeleruptorModule(const Bus& bus, const uint8_t address, const uint16_t initialData)
    : ModuleBase(bus, address, ModuleType::Teleruptor)
    , m_numberOfTeleruptors(initialData & 0x0F)
{
    for (uint8_t i = 0; i < m_numberOfTeleruptors; ++i) {
        m_teleruptorPins.push_back(std::make_shared<InputPin<bool>>([this, i](bool state) { UpdateTeleruptorState(i, state); }, false));
        m_teleruptorFeedbackPins.push_back(std::make_shared<OutputPin<bool>>(false));
    }
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
            m_teleruptorFeedbackPins[i]->SetState((response.Data & (1 << i)) != 0);
        }

        m_feedbackStateInSync = true;
    }

    return { .Success = true };
}

std::vector<std::weak_ptr<InputPin<bool>>> TeleruptorModule::GetDigitalInputPins() const
{
    std::vector<std::weak_ptr<InputPin<bool>>> inputPins;
    for (const auto& pin : m_teleruptorPins) {
        inputPins.push_back(pin);
    }

    return inputPins;
}

std::vector<std::weak_ptr<OutputPin<bool>>> TeleruptorModule::GetDigitalOutputPins() const
{
    std::vector<std::weak_ptr<OutputPin<bool>>> outputPins;
    for (const auto& pin : m_teleruptorFeedbackPins) {
        outputPins.push_back(pin);
    }

    return outputPins;
}

void TeleruptorModule::UpdateTeleruptorState(const uint8_t teleruptorIndex, const bool newState)
{
    uint16_t command = newState ? 0x01 : 0x02; // CMD1 - Set teleruptor ON, CMD2 - Set teleruptor OFF
    Exchange(command | (teleruptorIndex << 4));
}
