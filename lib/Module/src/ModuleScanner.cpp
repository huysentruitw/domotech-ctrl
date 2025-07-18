#include <memory>

#ifndef NATIVE_BUILD
#include "freertos/FreeRTOS.h"
#endif

#include "ModuleFactory.h"
#include "ModuleScanner.h"

ModuleScanner::ModuleScanner(Bus& bus)
    : m_bus(bus)
{
}

std::vector<std::unique_ptr<ModuleBase>> ModuleScanner::DetectModules()
{
    std::vector<std::unique_ptr<ModuleBase>> foundModules;

    for (uint8_t address = 1; address < 128; address++) {
        auto response = m_bus.Exchange(address, 0);

        if (!response.Success || !response.RespondedWithTypeAndData) {
            continue;
        }

        auto module = ModuleFactory::CreateModule(m_bus, (ModuleType)response.ModuleType, address, response.Data);

        if (module == nullptr) {
            // Module type not recognized, skip this address
            continue;
        }

        foundModules.push_back(std::move(module));

#ifndef NATIVE_BUILD        
        vTaskDelay(1);  // Let other tasks run, then resume
#endif
    }

    return foundModules;
}
