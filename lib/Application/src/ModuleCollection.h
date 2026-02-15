#pragma once

#include <Bus.h>
#include <IStorage.h>
#include <Lock.h>
#include <Module.h>

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

using ModuleFactoryFn = std::function<std::unique_ptr<Module>(ModuleType /*type*/, uint8_t /*address*/, uint16_t /*initialData*/)>;

class ModuleCollection final
{
public:
    ModuleCollection(IStorage& storage, std::string_view fileName) noexcept;

    bool LoadFromFile(const ModuleFactoryFn& factory) noexcept;
    bool Emplace(std::vector<std::unique_ptr<Module>> modules) noexcept;

    size_t Count() const noexcept;
    std::shared_ptr<Module> TryGetModuleByAddress(uint8_t address) const noexcept;

    auto begin() const noexcept { return m_modules.begin(); }
    auto end() const noexcept { return m_modules.end(); }

private:
    IStorage& m_storage;
    std::string m_fileName;
    std::vector<std::shared_ptr<Module>> m_modules;

    bool SaveToFile() const noexcept;
};
