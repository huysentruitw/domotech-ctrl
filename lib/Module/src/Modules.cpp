#include "Modules.h"

// PushButtonModule
PushButtonModule::PushButtonModule(const uint8_t address, const uint16_t initialData)
    : ModuleBase(address, ModuleType::PushButtons)
    , numberOfButtons((initialData >> 12) & 0x0F)
{
}

bool PushButtonModule::Process(const BusProtocol& bus) const
{
    auto response = this->Poll(bus);
    return response.Success;
}

// RelaisPulseModule
RelaisPulseModule::RelaisPulseModule(const uint8_t address, const uint16_t initialData)
    : ModuleBase(address, ModuleType::RelaisPulse)
    , numberOfRelais(initialData & 0x0F)
{
}

bool RelaisPulseModule::Process(const BusProtocol& bus) const
{
    auto response = this->Poll(bus);
    return response.Success;
}
