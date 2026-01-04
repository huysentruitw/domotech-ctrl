#pragma once

#include "RescanModulesResult.h"
#include "ScanLed.h"

#include <Filter.h>
#include <Lock.h>
#include <Module.h>
#include <StringHash.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Manager final
{
public:
    Manager();

    void Start();
    void ProcessNext();

    void Clear();
    RescanModulesResult RescanModules();

    bool TryCreateFilter(std::string_view typeName, std::string_view name);

    std::string GetKnownFiltersIni() const;
    std::string GetConfigurationIni() const;

private:
    const BusDriver m_busDriver;
    const ScanLed m_scanLed;
    const Bus m_bus;
    const Lock m_syncRoot;

    std::size_t m_nextModuleIndexToProcess = 0;

    std::unordered_map<std::string, std::shared_ptr<Filter>, StringHash, std::equal_to<>> m_filters;
    std::vector<std::shared_ptr<Module>> m_modules;
};
