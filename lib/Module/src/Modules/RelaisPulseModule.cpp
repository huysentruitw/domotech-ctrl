#include "RelaisPulseModule.h"

RelaisPulseModule::RelaisPulseModule(const uint8_t address, const uint16_t initialData)
    : ModuleBase(address, ModuleType::RelaisPulse)
    , numberOfRelais(initialData & 0x0F)
{
    for (uint8_t i = 0; i < this->numberOfRelais; ++i)
    {
        this->relaisPins.push_back(std::make_shared<InputPin<bool>>(false));
        this->relaisFeedbackPins.push_back(std::make_shared<OutputPin<bool>>(false));
    }
}

bool RelaisPulseModule::Process(const BusProtocol& bus)
{
    auto response = this->Poll(bus);
    return response.Success;
}

std::vector<std::weak_ptr<InputPin<bool>>> RelaisPulseModule::GetDigitalInputPins() const
{
    std::vector<std::weak_ptr<InputPin<bool>>> inputPins;
    for (const auto& pin : relaisPins)
        inputPins.push_back(pin);
    return inputPins;
}

std::vector<std::weak_ptr<OutputPin<bool>>> RelaisPulseModule::GetDigitalOutputPins() const
{
    std::vector<std::weak_ptr<OutputPin<bool>>> outputPins;
    for (const auto& pin : relaisFeedbackPins)
        outputPins.push_back(pin);
    return outputPins;
}
