#include "Manager.h"
#include "ConnectionsParser.h"

#include <IniReader.h>
#include <IniWriter.h>
#include <LockGuard.h>
#include <ModuleFactory.h>
#include <ModuleScanner.h>

#include <FilterFactory.h>
#include <Filters/ClimateFilter.h>
#include <Filters/DigitalPassthroughFilter.h>
#include <Filters/DimmerFilter.h>
#include <Filters/LightFilter.h>
#include <Filters/ShutterFilter.h>
#include <Filters/SwitchFilter.h>

#include <sstream>

#define MODULES_FILE_NAME "Modules.ini"
#define FILTER_CONFIG_FILE_NAME "FilterConfig.ini"

Manager::Manager(IStorage* const storage, IBridge* const bridge) noexcept
    : m_storage(storage)
    , m_bridge(bridge)
    , m_busDriver()
    , m_bus(m_busDriver)
    , m_syncRoot()
{
}

void Manager::Start() noexcept
{
    m_busDriver.Init();
    LoadModulesFromFile();
}

void Manager::ProcessNext() noexcept
{
    if (m_tick % 8 == 0)
        m_scanLed.Toggle();

    LockGuard guard(m_syncRoot);
    for (const auto& module : m_modules)
        module->Tick(m_tick);

    m_tick++;
}

void Manager::Clear() noexcept
{
    {
        LockGuard guard(m_syncRoot);
        m_filtersById.clear();
        m_modules.clear();
    }

    SaveModulesToFile();
}

RescanModulesResult Manager::RescanModules() noexcept
{
    {
        LockGuard guard(m_syncRoot);

        m_filtersById.clear();
        m_modules.clear();

        const ModuleScanner scanner(m_bus);
        auto detectedModules = scanner.DetectModules();

        m_modules.reserve(detectedModules.size());
        for (auto& module : detectedModules)
            m_modules.emplace_back(std::shared_ptr(std::move(module)));
    }

    SaveModulesToFile();

    return
    {
        .NumberOfDetectedModules = m_modules.size(),
    };
}

void Manager::SaveModulesToFile() noexcept
{
    LockGuard guard(m_syncRoot);
    IniWriter iniWriter;

    for (const auto& module : m_modules)
    {
        iniWriter.WriteSection("Module");
        iniWriter.WriteKeyValue("Type", std::to_string((int)module->GetType()));
        iniWriter.WriteKeyValue("Address", std::to_string(module->GetAddress()));
        iniWriter.WriteKeyValue("InitialData", std::to_string(module->GenerateInitialData()));
    }

    m_storage->WriteFile(MODULES_FILE_NAME, iniWriter.GetContent());
}

void Manager::LoadModulesFromFile() noexcept
{
    LockGuard guard(m_syncRoot);
    m_filtersById.clear();
    m_modules.clear();

    std::string type;
    std::string address;
    std::string initialData;

    type.reserve(2);
    address.reserve(3);
    initialData.reserve(5);

    IniReader iniReader;

    iniReader.OnSection([&](std::string_view section)
    {
        type.clear();
        address.clear();
        initialData.clear();
    });

    iniReader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value)
    {
        if (key == "Type") type = value;
        else if (key == "Address") address = value;
        else if (key == "InitialData") initialData = value;

        if (!type.empty() && !address.empty() && !initialData.empty())
        {
            std::unique_ptr<Module> module = ModuleFactory::CreateModule(
                m_bus,
                static_cast<ModuleType>(std::stoi(type)),
                static_cast<uint8_t>(std::stoi(address)),
                static_cast<uint16_t>(std::stoi(initialData)));

            m_modules.emplace_back(std::shared_ptr(std::move(module)));
        }
    });

    m_storage->ReadFileInChunks(MODULES_FILE_NAME, [&](const char* chunk, size_t chunkSize)
    {
        iniReader.Feed(chunk, chunkSize);
    });

    iniReader.Finalize();
}

bool Manager::TryCreateFilter(std::string_view typeName, std::string_view id, std::string_view connections) noexcept
{
    LockGuard guard(m_syncRoot);

    if (TryGetFilterById(id) != nullptr)
        return false;

    const auto connectionsResult = ParseConnections<8>(connections);
    if (!connectionsResult.ok)
        return false;

    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(typeName, id));
    if (filter == nullptr)
        return false;

    m_filtersById.emplace(filter->GetId(), filter);

    if (m_bridge != nullptr)
        m_bridge->RegisterAsDevice(filter);

    // Apply connections
    for (size_t i = 0; i < connectionsResult.count; i++)
    {
        const auto& mapping = connectionsResult.mappings[i];

        if (mapping.LocalPin.Direction == mapping.RemotePin.Direction)
            continue;

        const auto& filterPins = mapping.LocalPin.Direction == PinDirection::Input ? filter->GetInputPins() : filter->GetOutputPins();
        if (mapping.LocalPin.Index >= filterPins.size())
            continue;

        const auto remoteModule = TryGetModuleByAddress(mapping.RemoteModule.Address);
        if (remoteModule == nullptr)
            continue;

        const auto& remotePins = mapping.RemotePin.Direction == PinDirection::Input ? remoteModule->GetInputPins() : remoteModule->GetOutputPins();
        if (mapping.RemotePin.Index >= remotePins.size())
            continue;

        const auto& inputPin = mapping.LocalPin.Direction == PinDirection::Input ? filterPins[mapping.LocalPin.Index] : remotePins[mapping.RemotePin.Index];
        const auto& outputPin = mapping.LocalPin.Direction == PinDirection::Input ? remotePins[mapping.RemotePin.Index] : filterPins[mapping.LocalPin.Index];
        Pin::Connect(inputPin, outputPin);
    }

    return true;
}

std::string Manager::ReadModulesIniFile() const noexcept
{
    std::string result;

    bool ok = m_storage->ReadFileInChunks(
        MODULES_FILE_NAME,
        [&](const char* data, size_t size)
        {
            result.append(data, size);
        });

    return ok ? result : std::string{};

    // IniWriter iniWriter;

    // {
    //     LockGuard guard(m_syncRoot);

    //     for (const auto& module : m_modules)
    //         module->WriteConfig(iniWriter);

    //     for (const auto& [id, filter] : m_filtersById)
    //         filter->WriteConfig(iniWriter);
    // }

    // return iniWriter.GetContent();
}

std::shared_ptr<Filter> Manager::TryGetFilterById(std::string_view id) const noexcept
{
    auto it = m_filtersById.find(id);
    if (it == m_filtersById.end())
        return nullptr;

    return it->second;
}

std::shared_ptr<Module> Manager::TryGetModuleByAddress(uint8_t address) const noexcept
{
    for (const auto& module : m_modules)
    {
        if (module->GetAddress() == address)
            return module;
    }

    return nullptr;
}
