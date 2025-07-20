#include "TeleruptorModule.h"

#include <sstream>
#include <string>
#include <KnownModuleIdentifiers.h>

TeleruptorModule::TeleruptorModule(const Bus& bus, const uint8_t address, const uint8_t numberOfTeleruptors)
    : ModuleBase(bus, address, ModuleType::Teleruptor)
    , m_numberOfTeleruptors(numberOfTeleruptors)
{
    for (uint8_t i = 0; i < m_numberOfTeleruptors; ++i) {
        m_teleruptorPins.push_back(std::make_shared<InputPin<DigitalValue>>([this, i](DigitalValue value) { UpdateTeleruptor(i, value); }, DigitalValue(false)));
        m_teleruptorFeedbackPins.push_back(std::make_shared<OutputPin<DigitalValue>>(DigitalValue(false)));
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

std::vector<std::weak_ptr<InputPin<DigitalValue>>> TeleruptorModule::GetDigitalInputPins() const
{
    std::vector<std::weak_ptr<InputPin<DigitalValue>>> inputPins;
    for (const auto& pin : m_teleruptorPins) {
        inputPins.push_back(pin);
    }

    return inputPins;
}

std::vector<std::weak_ptr<OutputPin<DigitalValue>>> TeleruptorModule::GetDigitalOutputPins() const
{
    std::vector<std::weak_ptr<OutputPin<DigitalValue>>> outputPins;
    for (const auto& pin : m_teleruptorFeedbackPins) {
        outputPins.push_back(pin);
    }

    return outputPins;
}

void TeleruptorModule::UpdateTeleruptor(const uint8_t teleruptorIndex, const DigitalValue newValue)
{
    uint16_t command = newValue ? 0x01 : 0x02; // CMD1 - Set teleruptor ON, CMD2 - Set teleruptor OFF
    Exchange(command | (teleruptorIndex << 4));
}
