#include "Manager.h"
#include "ConnectionsParser.h"
#include "NumberUtilities.h"

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
#define FILTERS_FILE_NAME "Filter.ini"

Manager::Manager(IStorage* const storage, IBridge* const bridge) noexcept
    : m_storage(storage)
    , m_bridge(bridge)
    , m_busDriver()
    , m_bus(m_busDriver)
    , m_syncRoot(true)
{
}

void Manager::Start() noexcept
{
    m_busDriver.Init();

    // LoadModulesFromFile();
    // LoadFiltersFromFile();
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

    m_storage->RemoveFile(MODULES_FILE_NAME);
    m_storage->RemoveFile(FILTERS_FILE_NAME);
}

RescanModulesResult Manager::RescanModules() noexcept
{
    Clear();

    LockGuard guard(m_syncRoot);

    const ModuleScanner scanner(m_bus);
    auto detectedModules = scanner.DetectModules();

    m_modules.reserve(detectedModules.size());
    for (auto& module : detectedModules)
        m_modules.emplace_back(std::shared_ptr(std::move(module)));

    SaveModulesToFile();

    return
    {
        .NumberOfDetectedModules = m_modules.size(),
    };
}

void Manager::SaveModulesToFile() noexcept
{
    IniWriter iniWriter;

    {
        LockGuard guard(m_syncRoot);
        for (const auto& module : m_modules)
        {
            iniWriter.WriteSection("Module");
            iniWriter.WriteKeyValue("Type", GetModuleTypeName(module->GetType()));
            iniWriter.WriteKeyValue("Address", std::to_string(module->GetAddress()));
            iniWriter.WriteKeyValue("InitialData", ToHex4(module->GenerateInitialData()));
        }
    }

    m_storage->WriteFile(MODULES_FILE_NAME, iniWriter.GetContent());
}

void Manager::LoadModulesFromFile() noexcept
{
    IniReader iniReader;

    bool inModuleSection;
    std::optional<ModuleType> type;
    std::optional<uint8_t> address;
    std::optional<uint16_t> initialData;

    iniReader.OnSection([&](std::string_view section)
    {
        inModuleSection = (section == "Module");
        type = std::nullopt;
        address = std::nullopt;
        initialData = std::nullopt;
    });

    std::vector<std::unique_ptr<Module>> modules;
    iniReader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value)
    {
        if (!inModuleSection)
            return;

        if (key == "Type") type = GetModuleType(value);
        else if (key == "Address") address = ParseInt(value);
        else if (key == "InitialData") initialData = ParseInt(value, 16);

        if (type && address && initialData)
        {
            modules.emplace_back(ModuleFactory::CreateModule(m_bus, *type, *address, *initialData));
        }
    });

    if (!m_storage->ReadFileInChunks(
        MODULES_FILE_NAME,
        [&](const char* chunk, size_t chunkSize)
        {
            iniReader.Feed(chunk, chunkSize);
        }))
    {
        return;
    }

    iniReader.Finalize();

    LockGuard guard(m_syncRoot);
    m_modules.reserve(modules.size());
    for (auto& module : modules)
        m_modules.emplace_back(std::shared_ptr(std::move(module)));
}

void Manager::AppendFilterToFile(std::string_view id, std::string_view typeName, std::string_view connections) noexcept
{
    IniWriter iniWriter;
    iniWriter.WriteSection("Filter");
    iniWriter.WriteKeyValue("Id", id);
    iniWriter.WriteKeyValue("Type", typeName);
    iniWriter.WriteKeyValue("Connections", connections);
    
    m_storage->AppendFile(FILTERS_FILE_NAME, iniWriter.GetContent());
    m_storage->AppendFile(FILTERS_FILE_NAME, "\n");
}

void Manager::LoadFiltersFromFile() noexcept
{
    IniReader iniReader;

    bool inFilterSection;
    std::string id;
    std::string typeName;
    std::string connections;

    iniReader.OnSection([&](std::string_view section)
    {
        inFilterSection = (section == "Filter");
        id.clear();
        typeName.clear();
        connections.clear();
    });

    iniReader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value)
    {
        if (!inFilterSection)
            return;

        if (key == "Id") id = value;
        else if (key == "Type") typeName = value;
        else if (key == "Connections") connections = value;

        if (!id.empty() && !typeName.empty() && !connections.empty())
            CreateFilter(id, typeName, connections);
    });

    if (!m_storage->ReadFileInChunks(
        FILTERS_FILE_NAME,
        [&](const char* chunk, size_t chunkSize)
        {
            iniReader.Feed(chunk, chunkSize);
        }))
    {
        return;
    }

    iniReader.Finalize();
}

CreateFilterResult Manager::CreateFilter(std::string_view id, std::string_view typeName, std::string_view connections) noexcept
{
    LockGuard guard(m_syncRoot);

    CreateFilterResult result{};
    auto filter = Manager::CreateFilterInternal(id, typeName, connections, result);
    if (result.Status != CreateFilterStatus::NoError || filter == nullptr)
        return result;

    // Register filter
    m_filtersById.emplace(filter->GetId(), filter);

    AppendFilterToFile(id, typeName, connections);

    if (m_bridge != nullptr)
        m_bridge->RegisterAsDevice(filter);

    return result;
}

std::string Manager::ReadModulesIniFile() const noexcept
{
    std::string result;

    m_storage->ReadFileInChunks(
        MODULES_FILE_NAME,
        [&](const char* data, size_t size)
        {
            result.append(data, size);
        });

    bool ok = m_storage->ReadFileInChunks(
        FILTERS_FILE_NAME,
        [&](const char* data, size_t size)
        {
            result.append(data, size);
        });

    return ok ? result : std::string{};
}

std::shared_ptr<Filter> Manager::CreateFilterInternal(std::string_view id, std::string_view typeName, std::string_view connections, CreateFilterResult& result)
{
    std::vector<std::pair<std::weak_ptr<Pin>, std::weak_ptr<Pin>>> pinConnections;
    auto fail = [&](CreateFilterStatus status, std::optional<size_t> failedAtMappingIndex = std::nullopt) -> std::shared_ptr<Filter>
    {
        result.Status = status;
        result.FailedAtMappingIndex = failedAtMappingIndex;
        for (const auto& pinConnection : pinConnections)
            Pin::Disconnect(pinConnection.first, pinConnection.second);
        return nullptr;
    };

    if (TryGetFilterById(id) != nullptr)
        return fail(CreateFilterStatus::FilterAlreadyExists);

    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(typeName, id));
    if (filter == nullptr)
        return fail(CreateFilterStatus::UnknownFilterType);

    const auto mappings = TryParseConnections<8>(connections);
    if (!mappings)
        return fail(CreateFilterStatus::FailedToParseConnections);

    pinConnections.reserve((*mappings).Count());

    for (size_t i = 0; i < (*mappings).Count(); i++)
    {
        const auto& mapping = (*mappings)[i];

        if (mapping.LocalPin.Direction == mapping.RemotePin.Direction)
            return fail(CreateFilterStatus::PinDirectionsMismatch, i);

        const auto remoteModule = TryGetModuleByAddress(mapping.RemoteModule.Address);
        if (remoteModule == nullptr)
            return fail(CreateFilterStatus::UnknownRemoteModule, i);

        const auto& filterPins = mapping.LocalPin.Direction == PinDirection::Input ? filter->GetInputPins() : filter->GetOutputPins();
        if (mapping.LocalPin.Index >= filterPins.size())
            return fail(CreateFilterStatus::LocalPinIndexOutOfRange, i);

        const auto& remotePins = mapping.RemotePin.Direction == PinDirection::Input ? remoteModule->GetInputPins() : remoteModule->GetOutputPins();
        if (mapping.RemotePin.Index >= remotePins.size())
            return fail(CreateFilterStatus::RemotePinIndexOutOfRange, i);

        const auto inputPin = mapping.LocalPin.Direction == PinDirection::Input ? filterPins[mapping.LocalPin.Index] : remotePins[mapping.RemotePin.Index];
        const auto outputPin = mapping.LocalPin.Direction == PinDirection::Input ? remotePins[mapping.RemotePin.Index] : filterPins[mapping.LocalPin.Index];

        if (!Pin::Connect(inputPin, outputPin))
            return fail(CreateFilterStatus::PinConnectionFailed, i);

        pinConnections.emplace_back(inputPin, outputPin);
    }

    result.Status = CreateFilterStatus::NoError;
    return filter;
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
