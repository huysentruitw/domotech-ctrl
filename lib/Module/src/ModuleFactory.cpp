#include "ModuleFactory.h"

#include "Modules/DimmerModule.h"
#include "Modules/InputModule.h"
#include "Modules/PushButtonLedModule.h"
#include "Modules/PushButtonModule.h"
#include "Modules/PushButtonTemperatureModule.h"
#include "Modules/TeleruptorModule.h"

ModuleFactory::ModuleFactory(Bus& bus)
    : m_bus(bus)
{
}

std::unique_ptr<Module> ModuleFactory::CreateModule(const ModuleType type, const uint8_t address, const uint16_t initialData) const noexcept
{
    if (type == ModuleType::Dimmer) return DimmerModule::CreateFromInitialData(m_bus, address, initialData);
    if (type == ModuleType::PushButton) return PushButtonModule::CreateFromInitialData(m_bus, address, initialData);
    if (type == ModuleType::PushButtonWithLed) return PushButtonLedModule::CreateFromInitialData(m_bus, address, initialData);
    if (type == ModuleType::PushButtonWithTemperature) return PushButtonTemperatureModule::CreateFromInitialData(m_bus, address, initialData);
    if (type == ModuleType::Teleruptor) return TeleruptorModule::CreateFromInitialData(m_bus, address, initialData);
    if (type == ModuleType::Input) return InputModule::CreateFromInitialData(m_bus, address, initialData);
    return nullptr;
}
