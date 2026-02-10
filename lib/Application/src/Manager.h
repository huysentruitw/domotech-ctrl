#pragma once

#include "RescanModulesResult.h"
#include "ScanLed.h"

#include <Filter.h>
#include <IBridge.h>
#include <IStorage.h>
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
    Manager(IStorage* const storage, IBridge* const bridge = nullptr) noexcept;

    void Start() noexcept;
    void ProcessNext() noexcept;

    void Clear() noexcept;
    RescanModulesResult RescanModules() noexcept;

    bool TryCreateFilter(std::string_view typeName, std::string_view id, std::string_view connections) noexcept;

    std::string ReadModulesIniFile() const noexcept;

private:
    void SaveModulesToFile() noexcept;
    void LoadModulesFromFile() noexcept;

private:
    IStorage* const m_storage;
    IBridge* const m_bridge = nullptr;

    BusDriver m_busDriver;
    ScanLed m_scanLed;
    Bus m_bus;
    Lock m_syncRoot;

    uint16_t m_tick = 0;

    std::unordered_map<std::string, std::shared_ptr<Filter>, StringHash, std::equal_to<>> m_filtersById;
    std::vector<std::shared_ptr<Module>> m_modules;

    std::shared_ptr<Filter> TryGetFilterById(std::string_view id) const noexcept;
    std::shared_ptr<Module> TryGetModuleByAddress(uint8_t address) const noexcept;
};
