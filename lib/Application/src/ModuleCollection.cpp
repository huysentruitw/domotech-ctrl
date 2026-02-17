#include "ModuleCollection.h"

#include "NumberUtilities.h"

#include <IniReader.h>
#include <IniWriter.h>
#include <LockGuard.h>

ModuleCollection::ModuleCollection(IStorage& storage, std::string_view fileName) noexcept
    : m_storage(storage)
    , m_fileName(fileName)
{
}

bool ModuleCollection::LoadFromFile(const ModuleFactoryFn& factory) noexcept
{
    IniReader iniReader;

    std::optional<ModuleType> type;
    std::optional<uint8_t> address;
    std::optional<uint16_t> initialData;

    iniReader.OnSection([&](std::string_view section)
    {
        type = std::nullopt;
        address = std::nullopt;
        initialData = std::nullopt;
    });

    std::vector<std::unique_ptr<Module>> modules;
    iniReader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value)
    {
        if (section != "Module")
            return;

        if (key == "Type") type = GetModuleType(value);
        else if (key == "Address") address = NumberUtilities::ParseInt(value);
        else if (key == "InitialData") initialData = NumberUtilities::ParseInt(value, 16);

        if (type && address && initialData)
        {
            if (auto module = factory(type.value(), address.value(), initialData.value()))
                modules.push_back(std::move(module));
        }
    });

    if (!m_storage.ReadFileInChunks(
        m_fileName,
        [&](const char* chunk, size_t chunkSize)
        {
            iniReader.Feed(chunk, chunkSize);
            return true;
        }))
    {
        return false;
    }

    iniReader.Finalize();

    m_modules.assign(
        std::make_move_iterator(modules.begin()),
        std::make_move_iterator(modules.end()));

    return true;
}

bool ModuleCollection::Emplace(std::vector<std::unique_ptr<Module>> modules) noexcept
{
    m_modules.assign(
        std::make_move_iterator(modules.begin()),
        std::make_move_iterator(modules.end()));

    return SaveToFile();
}

size_t ModuleCollection::Count() const noexcept
{
    return m_modules.size();
}

std::shared_ptr<Module> ModuleCollection::TryGetModuleByAddress(uint8_t address) const noexcept
{
    for (const auto& module : m_modules)
    {
        if (module->GetAddress() == address)
            return module;
    }

    return nullptr;
}

bool ModuleCollection::SaveToFile() const noexcept
{
    IniWriter iniWriter;

    for (const auto& module : m_modules)
    {
        iniWriter.WriteSection("Module");
        iniWriter.WriteKeyValue("Type", GetModuleTypeName(module->GetType()));
        iniWriter.WriteKeyValue("Address", std::to_string(module->GetAddress()));
        iniWriter.WriteKeyValue("InitialData", NumberUtilities::ToHex4(module->GenerateInitialData()));
    }

    return m_storage.WriteFile(m_fileName, iniWriter.GetContent());
}
