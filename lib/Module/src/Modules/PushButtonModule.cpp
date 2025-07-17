#include "PushButtonModule.h"

static const uint8_t ButtonMasks8[8] = { 0x10, 0x20, 0x40, 0x80, 0x08, 0x04, 0x02, 0x01 };

PushButtonModule::PushButtonModule(const Bus& bus, const uint8_t address, const uint16_t initialData)
    : ModuleBase(bus, address, ModuleType::PushButtons)
    , numberOfButtons((initialData >> 12) & 0x0F)
{
    for (uint8_t i = 0; i < this->numberOfButtons; ++i)
        this->buttonPins.push_back(std::make_shared<OutputPin<bool>>(false));
}

ProcessResponse PushButtonModule::Process()
{
    auto response = this->hasPressedButtons
        ? this->Exchange(0x06) // CMD6 - Request button state
        : this->Poll();

    if (!response.Success)
        return { .Success = false };

    if (response.RespondedWithTypeAndData)
    {
        for (uint8_t i = 0; i < this->numberOfButtons; ++i)
            this->buttonPins[i]->SetState(this->MapButtonState(i, response.Data));

        this->hasPressedButtons = (response.Data & 0x0FFF) != 0;
        return { .Success = true, .RaisePriority = this->hasPressedButtons };
    }

    if (this->hasPressedButtons)
    {
        this->hasPressedButtons = false;
        for (const auto& pin : this->buttonPins)
            pin->SetState(false);
    }

    return { .Success = true };
}

std::vector<std::weak_ptr<OutputPin<bool>>> PushButtonModule::GetDigitalOutputPins() const
{
    std::vector<std::weak_ptr<OutputPin<bool>>> outputPins;
    for (const auto& pin : buttonPins)
        outputPins.push_back(pin);

    return outputPins;
}

bool PushButtonModule::MapButtonState(const uint8_t buttonIndex, const uint16_t data) const
{
    if (this->numberOfButtons == 8)
        return (data & ButtonMasks8[buttonIndex]) != 0;

    return false;
}
