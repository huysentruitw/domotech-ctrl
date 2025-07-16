#include "ModuleFactory.h"
#include "Modules.h"

std::unique_ptr<ModuleBase> ModuleFactory::CreateModule(const ModuleType type, const uint8_t address, const uint16_t initialData)
{
    switch (type)
    {
        case ModuleType::PushButtons:
            return std::make_unique<PushButtonModule>(address, initialData);
        case ModuleType::RelaisPulse:
            return std::make_unique<RelaisPulseModule>(address, initialData);
        default:
            return nullptr;
    };
}
