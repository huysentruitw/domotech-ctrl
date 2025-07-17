#include "TeleruptorModule.h"

TeleruptorModule::TeleruptorModule(const Bus& bus, const uint8_t address, const uint16_t initialData)
    : ModuleBase(bus, address, ModuleType::Teleruptor)
    , numberOfTeleruptors(initialData & 0x0F)
{
    for (uint8_t i = 0; i < this->numberOfTeleruptors; ++i)
    {
        this->teleruptorPins.push_back(std::make_shared<InputPin<bool>>([this, i](bool state) { this->UpdateTeleruptorState(i, state); }, false));
        this->teleruptorFeedbackPins.push_back(std::make_shared<OutputPin<bool>>(false));
    }
}

ProcessResponse TeleruptorModule::Process()
{
    auto response = this->Poll();

    if (!response.Success)
        return { .Success = false };

    // The module response with type and data when one or more relais feedback pins are changed
    // The response doesn't include the current state, so we need to request it separately
    if (response.RespondedWithTypeAndData || !this->feedbackStateInSync)
    {
        response = this->Exchange(0x06); // CMD6 - Request feedback state

        if (!response.Success || !response.RespondedWithTypeAndData)
            return { .Success = false };

        for (uint8_t i = 0; i < this->numberOfTeleruptors; ++i)
            this->teleruptorFeedbackPins[i]->SetState((response.Data & (1 << i)) != 0);

        this->feedbackStateInSync = true;
    }

    return { .Success = true };
}

std::vector<std::weak_ptr<InputPin<bool>>> TeleruptorModule::GetDigitalInputPins() const
{
    std::vector<std::weak_ptr<InputPin<bool>>> inputPins;
    for (const auto& pin : teleruptorPins)
        inputPins.push_back(pin);
    return inputPins;
}

std::vector<std::weak_ptr<OutputPin<bool>>> TeleruptorModule::GetDigitalOutputPins() const
{
    std::vector<std::weak_ptr<OutputPin<bool>>> outputPins;
    for (const auto& pin : teleruptorFeedbackPins)
        outputPins.push_back(pin);
    return outputPins;
}

void TeleruptorModule::UpdateTeleruptorState(const uint8_t teleruptorIndex, const bool newState)
{
    uint16_t command = newState ? 0x01 : 0x02; // CMD1 - Set teleruptor ON, CMD2 - Set teleruptor OFF
    this->Exchange(command | (teleruptorIndex << 4));
}
