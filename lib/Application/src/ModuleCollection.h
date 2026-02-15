#pragma once

#include <Bus.h>
#include <IStorage.h>
#include <Module.h>
#include <ModuleFactory.h>

#include <memory>
#include <string_view>
#include <vector>

class ModuleCollection final
{
public:
    ModuleCollection() noexcept;
    explicit ModuleCollection(std::vector<std::unique_ptr<Module>> modules) noexcept;

    static ModuleCollection LoadFromFile(IStorage& storage, std::string_view fileName, ModuleFactory& factory) noexcept;
    void SaveToFile(IStorage& storage, std::string_view fileName) const noexcept;

    std::shared_ptr<Module> TryGetModuleByAddress(uint8_t address) const noexcept;

    auto begin() const noexcept { return m_modules.begin(); }
    auto end() const noexcept { return m_modules.end(); }

private:
    std::vector<std::shared_ptr<Module>> m_modules;
};
