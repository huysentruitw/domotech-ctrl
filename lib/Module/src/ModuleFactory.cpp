#include "ModuleFactory.h"
#include "Modules/PushButtonModule.h"
#include "Modules/TeleruptorModule.h"

std::unique_ptr<ModuleBase> ModuleFactory::CreateModule(const Bus& bus, const ModuleType type, const uint8_t address, const uint16_t initialData)
{
    switch (type) {
        case ModuleType::PushButtons:
            return std::make_unique<PushButtonModule>(bus, address, initialData);
        case ModuleType::Teleruptor:
            return std::make_unique<TeleruptorModule>(bus, address, initialData);
        default:
            return nullptr;
    };
}

std::unique_ptr<ModuleBase> ModuleFactory::CreateModule(const Bus& bus, const std::string moduleString)
{
    if (auto module = PushButtonModule::ConstructFromString(bus, moduleString)) {
        return module;
    }

    if (auto module = TeleruptorModule::ConstructFromString(bus, moduleString)) {
        return module;
    }
    
    return nullptr;
}
