#pragma once

#include "RescanModulesResult.h"
#include "ScanLed.h"

#include <Filter.h>
#include <Bridge.h>
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
    Manager(Bridge* bridge = nullptr);

    void Start();
    void ProcessNext();

    void Clear();
    RescanModulesResult RescanModules();

    bool TryCreateFilter(std::string_view typeName, std::string_view id, std::string_view connections);

    std::string GetKnownFiltersIni() const;
    std::string GetConfigurationIni() const;

private:
    Bridge* m_bridge = nullptr;

    const BusDriver m_busDriver;
    const ScanLed m_scanLed;
    const Bus m_bus;
    const Lock m_syncRoot;

    std::size_t m_nextModuleIndexToProcess = 0;

    std::unordered_map<std::string, std::shared_ptr<Filter>, StringHash, std::equal_to<>> m_filtersById;
    std::vector<std::shared_ptr<Module>> m_modules;

    std::shared_ptr<Filter> TryGetFilterById(std::string_view id) const;
    std::shared_ptr<Module> TryGetModuleByAddress(uint8_t address) const;
};
