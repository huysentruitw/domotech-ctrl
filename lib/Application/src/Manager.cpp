#include "Manager.h"
#include "ConnectionsParser.h"

#include <FilterFactory.h>
#include <LockGuard.h>
#include <ModuleFactory.h>
#include <ModuleScanner.h>

#define MODULES_FILE_NAME "Modules.ini"
#define FILTERS_FILE_NAME "Filter.ini"

Manager::Manager(IStorage& storage, IBridge& bridge) noexcept
    : m_syncRoot(true)
    , m_storage(storage)
    , m_bridge(bridge)
    , m_busDriver()
    , m_bus(m_busDriver)
    , m_modules(storage, MODULES_FILE_NAME)
    , m_filters(storage, FILTERS_FILE_NAME)
{
}

void Manager::Start() noexcept
{
    m_busDriver.Init();

    m_modules.LoadFromFile(
        [&](ModuleType type, uint8_t address, uint16_t initialData)
        {
            return ModuleFactory::CreateModule(m_bus, type, address, initialData);
        });

    m_filters.LoadFromFile(
        [&](std::string_view id, std::string_view typeName, std::string_view connections)
        {
            CreateFilterResult result;
            return CreateFilterInternal(id, typeName, connections, result);
        });

    for (auto& [_, filter] : m_filters)
        m_bridge.RegisterAsDevice(filter);
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

    m_filters.Clear();

    ModuleScanner scanner(m_bus);
    auto detectedModules = scanner.DetectModules();

    m_modules.Emplace(std::move(detectedModules));

    return { .NumberOfDetectedModules = m_modules.Count() };
}

CreateFilterResult Manager::CreateFilter(std::string_view id, std::string_view typeName, std::string_view connections) noexcept
{
    LockGuard guard(m_syncRoot);

    CreateFilterResult result{};
    std::unique_ptr<Filter> filter = Manager::CreateFilterInternal(id, typeName, connections, result);
    if (result.Status != CreateFilterStatus::NoError || filter == nullptr)
        return result;

    auto storedFilter = m_filters.AppendFilter(std::move(filter), connections);
    m_bridge.RegisterAsDevice(storedFilter);

    return CreateFilterResult{};
}

std::unique_ptr<Filter> Manager::CreateFilterInternal(std::string_view id, std::string_view typeName, std::string_view connections, CreateFilterResult& result) noexcept
{
    std::vector<std::pair<std::weak_ptr<Pin>, std::weak_ptr<Pin>>> pinConnections;
    auto fail = [&](CreateFilterStatus status, std::optional<size_t> failedAtMappingIndex = std::nullopt) -> std::unique_ptr<Filter>
    {
        result.Status = status;
        result.FailedAtMappingIndex = failedAtMappingIndex;
        for (const auto& pinConnection : pinConnections)
            Pin::Disconnect(pinConnection.first, pinConnection.second);
        return nullptr;
    };

    if (m_filters.TryGetFilterById(id) != nullptr)
        return fail(CreateFilterStatus::FilterAlreadyExists);

    auto filter = FilterFactory::TryCreateFilterByTypeName(typeName, id);
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
