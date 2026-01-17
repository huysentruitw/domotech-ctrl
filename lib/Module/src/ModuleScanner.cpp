#include "ModuleScanner.h"
#include "ModuleFactory.h"

#include <memory>

#ifndef NATIVE_BUILD
    #include "freertos/FreeRTOS.h"
#endif

ModuleScanner::ModuleScanner(const Bus& bus) noexcept
    : m_bus(bus)
{
}

std::vector<std::unique_ptr<Module>> ModuleScanner::DetectModules() const noexcept
{
    std::vector<std::unique_ptr<Module>> foundModules;

    for (uint8_t address = 1; address < 128; address++)
    {
        auto response = m_bus.Exchange(address, 0, true);

        if (!response.Success || !response.RespondedWithTypeAndData)
            continue;

        auto module = ModuleFactory::CreateModule(m_bus, static_cast<ModuleType>(response.ModuleType), address, response.Data);

        if (module == nullptr)
            continue; // Module type not recognized, skip this address

        foundModules.push_back(std::move(module));

#ifndef NATIVE_BUILD        
        vTaskDelay(1);  // Let other tasks run, then resume
#endif
    }

    return foundModules;
}
