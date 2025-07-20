#include "ModuleFactory.h"

#include "Modules/DimmerModule.h"
#include "Modules/PushButtonModule.h"
#include "Modules/TeleruptorModule.h"

std::unique_ptr<ModuleBase> ModuleFactory::CreateModule(const Bus& bus, const ModuleType type, const uint8_t address, const uint16_t initialData)
{
    switch (type) {
        case ModuleType::Dimmer:
            return DimmerModule::CreateFromInitialData(bus, address, initialData);
        case ModuleType::PushButtons:
            return PushButtonModule::CreateFromInitialData(bus, address, initialData);
        case ModuleType::Teleruptor:
            return TeleruptorModule::CreateFromInitialData(bus, address, initialData);
        default:
            return nullptr;
    };
}
