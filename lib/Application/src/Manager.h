#pragma once

#include "RescanModulesResult.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <Filter.h>
#include <Module.h>

#ifndef NATIVE_BUILD
#include "freertos/FreeRTOS.h"
#endif

class Manager final
{
public:
    Manager();

    void Start();
    void ProcessNext();

    void Clear();
    RescanModulesResult RescanModules();

    void AddFilter(const std::shared_ptr<Filter> filter);

    std::string GetKnownFiltersIni() const;
    std::string GetConfigurationIni() const;

private:
    const BusDriver m_busDriver;
    const Bus m_bus;
#ifndef NATIVE_BUILD     
    const SemaphoreHandle_t m_syncRoot;
#endif
    std::size_t m_nextModuleIndexToProcess = 0;

    std::vector<std::shared_ptr<Filter>> m_filters;
    std::vector<std::shared_ptr<Module>> m_modules;
};
