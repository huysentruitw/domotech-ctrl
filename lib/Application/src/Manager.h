#pragma once

#include "RescanModulesResult.h"

#include <Filter.h>
#include <Lock.h>
#include <Module.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Manager final
{
public:
    Manager();

    void Start();
    void ProcessNext();

    void Clear();
    RescanModulesResult RescanModules();

    void AddFilter(std::unique_ptr<Filter> filter);

    std::string GetKnownFiltersIni() const;
    std::string GetConfigurationIni() const;

private:
    const BusDriver m_busDriver;
    const Bus m_bus;
    const Lock m_syncRoot;

    std::size_t m_nextModuleIndexToProcess = 0;

    std::vector<std::shared_ptr<Filter>> m_filters;
    std::vector<std::shared_ptr<Module>> m_modules;
};
