#include "Manager.h"
#include "KnownModuleIdentifiers.h"

#include <IniWriter.h>

#include <ModuleScanner.h>

#include <DimmerFilter.h>
#include <ShutterFilter.h>
#include <ToggleFilter.h>

#include <sstream>

Manager::Manager()
    : m_busDriver()
    , m_bus(m_busDriver)
#ifndef NATIVE_BUILD
    , m_syncRoot(xSemaphoreCreateRecursiveMutex())
#endif
{
}

void Manager::Start()
{
    m_busDriver.Init();
}

void Manager::ProcessNext()
{
#ifndef NATIVE_BUILD
    xSemaphoreTakeRecursive(m_syncRoot, portMAX_DELAY);
#endif

    if (m_modules.size() <= 0) {
#ifndef NATIVE_BUILD        
        xSemaphoreGiveRecursive(m_syncRoot);
#endif
        return;
    }

    if (m_nextModuleIndexToProcess >= m_modules.size())
        m_nextModuleIndexToProcess = 0;

    const auto& moduleToProcess = m_modules[m_nextModuleIndexToProcess++];
    
    const auto& _ = moduleToProcess->Process();

#ifndef NATIVE_BUILD
    xSemaphoreGiveRecursive(m_syncRoot);
#endif
}

void Manager::Clear()
{
#ifndef NATIVE_BUILD    
    xSemaphoreTakeRecursive(m_syncRoot, portMAX_DELAY);
#endif
    m_filters.clear();
    m_modules.clear();
    m_nextModuleIndexToProcess = 0;
#ifndef NATIVE_BUILD    
    xSemaphoreGiveRecursive(m_syncRoot);
#endif
}

RescanModulesResult Manager::RescanModules()
{
#ifndef NATIVE_BUILD
    xSemaphoreTakeRecursive(m_syncRoot, portMAX_DELAY);
#endif

    Clear();

    const ModuleScanner scanner(m_bus);
    
    const auto detectedModules = scanner.DetectModules();
    m_modules.insert(m_modules.end(), std::make_move_iterator(detectedModules.begin()), std::make_move_iterator(detectedModules.end()));

#ifndef NATIVE_BUILD
    xSemaphoreGiveRecursive(m_syncRoot);
#endif

    return {
        .NumberOfDetectedModules = (uint8_t)detectedModules.size(),
    };
}

std::string Manager::GetKnownFiltersIni() const
{
    const std::vector<std::shared_ptr<Filter>> filters = {
        std::make_shared<DimmerFilter>(),
        std::make_shared<ShutterFilter>(),
        std::make_shared<ToggleFilter>(),
    };

    auto iniWriter = IniWriter();

    for (const auto& filter : filters) {
        filter->WriteDescriptor(iniWriter);
    }

    return iniWriter.GetContent();
}

void Manager::AddFilter(const std::shared_ptr<Filter> filter)
{
    m_filters.push_back(filter);
}

std::string Manager::ToString() const
{   
    std::ostringstream result;

    result << "[Modules]" << std::endl;

    for (const auto& module : m_modules) {
        result << std::to_string(module->GetAddress()) << "=";

        switch (module->GetType()) {
            case ModuleType::Dimmer:
                result << KnownModuleIdentifiers::Dimmer << " " << std::to_string(module->GetInputPins().size());
                break;
            case ModuleType::Temperature:
                result << KnownModuleIdentifiers::Temperature;
                break;
            case ModuleType::Audio:
                result << KnownModuleIdentifiers::Audio;
                break;
            case ModuleType::PushButtons:
                result << KnownModuleIdentifiers::PushButtons << " " << std::to_string(module->GetOutputPins().size());
                break;
            case ModuleType::PushButtonsWithIr:
                result << KnownModuleIdentifiers::PushButtonsWithIr << " " << std::to_string(module->GetOutputPins().size());
                break;
            case ModuleType::PushButtonsWithTemperature:
                result << KnownModuleIdentifiers::PushButtonsWithTemperature << " " << std::to_string(module->GetOutputPins().size());
                break;
            case ModuleType::PushButtonsWithLeds:
                result << KnownModuleIdentifiers::PushButtonsWithLeds << " " << std::to_string(module->GetOutputPins().size());
                break;
            case ModuleType::Relais:
                result << KnownModuleIdentifiers::Relais << " " << std::to_string(module->GetInputPins().size());
                break;
            case ModuleType::Teleruptor:
                result << KnownModuleIdentifiers::Teleruptor << " " << std::to_string(module->GetInputPins().size());
                break;
            case ModuleType::Inputs:
                result << KnownModuleIdentifiers::Inputs << " " << std::to_string(module->GetInputPins().size());
                break;
            default:
                result << "UNK";
                break;
        }

        result << std::endl;
    }

    result << std::endl;

    result << "[Filters]" << std::endl;
    for (auto& _ : m_filters) {
        result << "FLT" << std::endl;
    }

    return result.str();
}
