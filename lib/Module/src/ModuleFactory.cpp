#include "ModuleFactory.h"

#include "Modules/DimmerModule.h"
#include "Modules/InputModule.h"
#include "Modules/PushButtonLedModule.h"
#include "Modules/PushButtonModule.h"
#include "Modules/PushButtonTemperatureModule.h"
#include "Modules/TeleruptorModule.h"

std::unique_ptr<Module> ModuleFactory::CreateModule(const Bus& bus, const ModuleType type, const uint8_t address, const uint16_t initialData) noexcept
{
    if (type == ModuleType::Dimmer) return DimmerModule::CreateFromInitialData(bus, address, initialData);
    if (type == ModuleType::PushButton) return PushButtonModule::CreateFromInitialData(bus, address, initialData);
    if (type == ModuleType::PushButtonWithLed) return PushButtonLedModule::CreateFromInitialData(bus, address, initialData);
    if (type == ModuleType::PushButtonWithTemperature) return PushButtonTemperatureModule::CreateFromInitialData(bus, address, initialData);
    if (type == ModuleType::Teleruptor) return TeleruptorModule::CreateFromInitialData(bus, address, initialData);
    if (type == ModuleType::Input) return InputModule::CreateFromInitialData(bus, address, initialData);
    return nullptr;
}
