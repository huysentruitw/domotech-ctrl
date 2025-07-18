#include "PushButtonModule.h"

static const uint8_t ButtonMasks8[8] = { 0x10, 0x20, 0x40, 0x80, 0x08, 0x04, 0x02, 0x01 };

PushButtonModule::PushButtonModule(const Bus& bus, const uint8_t address, const uint16_t initialData)
    : ModuleBase(bus, address, ModuleType::PushButtons)
    , m_numberOfButtons((initialData >> 12) & 0x0F)
{
    for (uint8_t i = 0; i < m_numberOfButtons; ++i) {
        m_buttonPins.push_back(std::make_shared<OutputPin<bool>>(false));
    }
}

ProcessResponse PushButtonModule::Process()
{
    auto response = m_hasPressedButtons
        ? Exchange(0x06) // CMD6 - Request button state
        : Poll();

    if (!response.Success) {
        return { .Success = false };
    }

    if (response.RespondedWithTypeAndData) {
        for (uint8_t i = 0; i < m_numberOfButtons; ++i) {
            m_buttonPins[i]->SetState(MapButtonState(i, response.Data));
        }

        m_hasPressedButtons = (response.Data & 0x0FFF) != 0;
        return { .Success = true, .RaisePriority = m_hasPressedButtons };
    }

    if (m_hasPressedButtons){
        m_hasPressedButtons = false;
        for (const auto& pin : m_buttonPins) {
            pin->SetState(false);
        }
    }

    return { .Success = true };
}

std::vector<std::weak_ptr<OutputPin<bool>>> PushButtonModule::GetDigitalOutputPins() const
{
    std::vector<std::weak_ptr<OutputPin<bool>>> outputPins;
    for (const auto& pin : m_buttonPins) {
        outputPins.push_back(pin);
    }

    return outputPins;
}

bool PushButtonModule::MapButtonState(const uint8_t buttonIndex, const uint16_t data) const
{
    if (m_numberOfButtons == 8) {
        return (data & ButtonMasks8[buttonIndex]) != 0;
    }

    return false;
}
