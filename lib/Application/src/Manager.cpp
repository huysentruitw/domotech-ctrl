#include "Manager.h"

#include <IniWriter.h>
#include <LockGuard.h>
#include <ModuleScanner.h>

#include <FilterFactory.h>
#include <Filters/DimmerFilter.h>
#include <Filters/ShutterFilter.h>
#include <Filters/ToggleFilter.h>

#include <sstream>

Manager::Manager()
    : m_busDriver()
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

bool Manager::TryCreateFilter(std::string_view typeName, std::string_view id, std::string_view name)
{
    LockGuard guard(m_syncRoot);

    if (TryGetFilterById(id) != nullptr)
        return false;

    auto filter = FilterFactory::TryCreateFilterByTypeName(typeName);
    if (filter == nullptr)
        return false;

    filter->SetName(name);
    m_filtersById.emplace(id, std::shared_ptr(std::move(filter)));
    return true;
}

std::string Manager::GetKnownFiltersIni() const
{
    IniWriter iniWriter;

    DimmerFilter().WriteDescriptor(iniWriter);
    ShutterFilter().WriteDescriptor(iniWriter);
    ToggleFilter().WriteDescriptor(iniWriter);

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
