#include "Manager.h"
#include "ConnectionsParser.h"

#include <IniWriter.h>
#include <LockGuard.h>
#include <ModuleScanner.h>

#include <FilterFactory.h>
#include <Filters/DimmerFilter.h>
#include <Filters/ShutterFilter.h>
#include <Filters/SwitchFilter.h>

#include <sstream>

Manager::Manager(Bridge* bridge)
    : m_bridge(bridge)
    , m_busDriver()
    , m_bus(m_busDriver)
    , m_syncRoot()
{
}

void Manager::Start()
{
    m_busDriver.Init();
}

void Manager::ProcessNext()
{
    std::shared_ptr<Module> moduleToProcess;

    {
        LockGuard guard(m_syncRoot);

        if (m_modules.empty()) {
            return;
        }

        if (m_nextModuleIndexToProcess >= m_modules.size())
        {
            m_nextModuleIndexToProcess = 0;
            m_scanLed.Toggle();
        }
        
        moduleToProcess = m_modules[m_nextModuleIndexToProcess++];
    }

    moduleToProcess->Process();
}

void Manager::Clear()
{
    LockGuard guard(m_syncRoot);

    m_filtersById.clear();
    m_modules.clear();
    m_nextModuleIndexToProcess = 0;
}

RescanModulesResult Manager::RescanModules()
{
    LockGuard guard(m_syncRoot);

    const ModuleScanner scanner(m_bus);
    auto detectedModules = scanner.DetectModules();

    m_filtersById.clear();
    m_modules.clear();
    m_nextModuleIndexToProcess = 0;

    m_modules.reserve(detectedModules.size());
    for (auto& module : detectedModules) {
        m_modules.emplace_back(std::shared_ptr(std::move(module)));
    }

    return {
        .NumberOfDetectedModules = m_modules.size(),
    };
}

bool Manager::TryCreateFilter(std::string_view typeName, std::string_view id, std::string_view connections)
{
    LockGuard guard(m_syncRoot);

    if (TryGetFilterById(id) != nullptr)
        return false;

    const auto connectionsResult = ParseConnections<8>(connections);
    if (!connectionsResult.ok)
        return false;

    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(typeName));
    if (filter == nullptr)
        return false;

    filter->SetName(id);
    m_filtersById.emplace(std::string(id), filter);

    if (m_bridge != nullptr)
        m_bridge->RegisterFilter(filter);

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

std::string Manager::GetKnownFiltersIni() const
{
    IniWriter iniWriter;

    SwitchFilter().WriteDescriptor(iniWriter);
    DimmerFilter().WriteDescriptor(iniWriter);
    ShutterFilter().WriteDescriptor(iniWriter);

    return iniWriter.GetContent();
}

std::string Manager::GetConfigurationIni() const
{
    IniWriter iniWriter;

    {
        LockGuard guard(m_syncRoot);

        for (const auto& module : m_modules) {
            module->WriteConfig(iniWriter);
        }

        for (const auto& [id, filter] : m_filtersById) {
            filter->WriteConfig(iniWriter, id);
        }
    }

    return iniWriter.GetContent();
}

std::shared_ptr<Filter> Manager::TryGetFilterById(std::string_view id) const
{
    auto it = m_filtersById.find(id);
    if (it == m_filtersById.end())
        return nullptr;

    return it->second;
}

std::shared_ptr<Module> Manager::TryGetModuleByAddress(uint8_t address) const
{
    for (const auto& module : m_modules) {
        if (module->GetAddress() == address)
            return module;
    }

    return nullptr;
}
