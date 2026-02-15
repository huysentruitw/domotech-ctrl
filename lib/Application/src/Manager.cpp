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
#define FILTERS_FILE_NAME "Filter.ini"

Manager::Manager(IStorage& storage, IBridge& bridge) noexcept
    : m_syncRoot(true)
    , m_storage(storage)
    , m_bridge(bridge)
    , m_busDriver()
    , m_bus(m_busDriver)
{
}

void Manager::Start() noexcept
{
    m_busDriver.Init();

    ModuleFactory moduleFactory(m_bus);
    m_modules = ModuleCollection::LoadFromFile(m_storage, MODULES_FILE_NAME, moduleFactory);
    m_filters = FilterCollection::LoadFromFile(m_storage, FILTERS_FILE_NAME);
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

RescanModulesResult Manager::RescanModules() noexcept
{
    LockGuard guard(m_syncRoot); // Prevent ProcessNext to interrupt this scanning process 

    m_filters = FilterCollection{};
    m_filters.SaveToFile(m_storage, FILTERS_FILE_NAME);

    ModuleFactory factory(m_bus);
    ModuleScanner scanner(m_bus);
    auto detectedModules = scanner.DetectModules();

    m_modules = ModuleCollection(std::move(detectedModules));
    m_modules.SaveToFile(m_storage, MODULES_FILE_NAME);

    return { .NumberOfDetectedModules = detectedModules.size() };
}

void Manager::AppendFilterToFile(std::string_view id, std::string_view typeName, std::string_view connections) noexcept
{
    // IniWriter iniWriter;
    // iniWriter.WriteSection("Filter");
    // iniWriter.WriteKeyValue("Id", id);
    // iniWriter.WriteKeyValue("Type", typeName);
    // iniWriter.WriteKeyValue("Connections", connections);
    
    // m_storage->AppendFile(FILTERS_FILE_NAME, iniWriter.GetContent());
    // m_storage->AppendFile(FILTERS_FILE_NAME, "\n");
}

void Manager::LoadFiltersFromFile() noexcept
{
    // IniReader iniReader;

    // bool inFilterSection;
    // std::string id;
    // std::string typeName;
    // std::string connections;

    // iniReader.OnSection([&](std::string_view section)
    // {
    //     inFilterSection = (section == "Filter");
    //     id.clear();
    //     typeName.clear();
    //     connections.clear();
    // });

    // iniReader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value)
    // {
    //     if (!inFilterSection)
    //         return;

    //     if (key == "Id") id = value;
    //     else if (key == "Type") typeName = value;
    //     else if (key == "Connections") connections = value;

    //     if (!id.empty() && !typeName.empty() && !connections.empty())
    //         CreateFilter(id, typeName, connections);
    // });

    // if (!m_storage->ReadFileInChunks(
    //     FILTERS_FILE_NAME,
    //     [&](const char* chunk, size_t chunkSize)
    //     {
    //         iniReader.Feed(chunk, chunkSize);
    //     }))
    // {
    //     return;
    // }

    // iniReader.Finalize();
}

CreateFilterResult Manager::CreateFilter(std::string_view id, std::string_view typeName, std::string_view connections) noexcept
{
    // LockGuard guard(m_syncRoot);

    // CreateFilterResult result{};
    // auto filter = Manager::CreateFilterInternal(id, typeName, connections, result);
    // if (result.Status != CreateFilterStatus::NoError || filter == nullptr)
    //     return result;

    // // Register filter
    // m_filtersById.emplace(filter->GetId(), filter);

    // AppendFilterToFile(id, typeName, connections);

    // if (m_bridge != nullptr)
    //     m_bridge->RegisterAsDevice(filter);

    // return result;
    return CreateFilterResult{};
}

std::shared_ptr<Filter> Manager::CreateFilterInternal(std::string_view id, std::string_view typeName, std::string_view connections, CreateFilterResult& result) noexcept
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

    if (m_filters.TryGetFilterById(id) != nullptr)
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

        const auto remoteModule = m_modules.TryGetModuleByAddress(mapping.RemoteModule.Address);
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
