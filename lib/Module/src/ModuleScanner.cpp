#include <memory>
#include "ModuleScanner.h"

#include "freertos/FreeRTOS.h"

ModuleScanner::ModuleScanner(BusProtocol& bus)
    : bus(bus)
{
}

std::vector<std::unique_ptr<ModuleBase>> ModuleScanner::DetectModules()
{
    std::vector<std::unique_ptr<ModuleBase>> foundModules;

    for (uint8_t address = 1; address < 128; address++)
    {
        auto response = bus.Exchange(address, 0, 3);

        if (!response.Success || !response.RespondedWithTypeAndData)
            continue;

        std::unique_ptr<ModuleBase> module;
        module = std::make_unique<ModuleBase>(address, response.ModuleType);
        foundModules.push_back(std::move(module));

        vTaskDelay(1);  // Let other tasks run, then resume
    }

    return foundModules;
}
