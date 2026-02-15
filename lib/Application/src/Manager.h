#pragma once

#include "CreateFilterResult.h"
#include "FilterCollection.h"
#include "ModuleCollection.h"
#include "RescanModulesResult.h"
#include "ScanLed.h"

#include <Filter.h>
#include <IBridge.h>
#include <IStorage.h>
#include <Lock.h>

#include <cstdint>
#include <memory>
#include <string_view>

class Manager final
{
public:
    Manager(IStorage& storage, IBridge& bridge) noexcept;

    void Start() noexcept;
    void ProcessNext() noexcept;

    RescanModulesResult RescanModules() noexcept;

    CreateFilterResult CreateFilter(std::string_view id, std::string_view typeName, std::string_view connections) noexcept;

private:
    void AppendFilterToFile(std::string_view id, std::string_view typeName, std::string_view connections) noexcept;
    void LoadFiltersFromFile() noexcept;

private:
    Lock m_syncRoot;
    IStorage& m_storage;
    IBridge& m_bridge;
    BusDriver m_busDriver;
    Bus m_bus;

    ScanLed m_scanLed;

    uint16_t m_tick = 0;

    ModuleCollection m_modules;
    FilterCollection m_filters;

    std::shared_ptr<Filter> CreateFilterInternal(std::string_view id, std::string_view typeName, std::string_view connections, CreateFilterResult& result) noexcept;
};
