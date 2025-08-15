#include "Manager.h"

#include <IniWriter.h>

#include <ModuleScanner.h>

#include <Filters/DimmerFilter.h>
#include <Filters/ShutterFilter.h>
#include <Filters/ToggleFilter.h>

#include <sstream>

#ifndef NATIVE_BUILD
 #define TAKE(mutex) xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
 #define RELEASE(mutex) xSemaphoreGiveRecursive(mutex);
#else
 #define TAKE(mutex)
 #define RELEASE(mutex)
#endif

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
    TAKE(m_syncRoot);

    if (m_modules.size() <= 0) {
        RELEASE(m_syncRoot);
        return;
    }

    if (m_nextModuleIndexToProcess >= m_modules.size())
        m_nextModuleIndexToProcess = 0;

    const auto& moduleToProcess = m_modules[m_nextModuleIndexToProcess++];
    
    const auto& _ = moduleToProcess->Process();

    RELEASE(m_syncRoot);
}

void Manager::Clear()
{
    TAKE(m_syncRoot);
    m_filters.clear();
    m_modules.clear();
    m_nextModuleIndexToProcess = 0;
    RELEASE(m_syncRoot);
}

RescanModulesResult Manager::RescanModules()
{
    TAKE(m_syncRoot);

    Clear();

    const ModuleScanner scanner(m_bus);
    
    const auto detectedModules = scanner.DetectModules();
    m_modules.insert(m_modules.end(), std::make_move_iterator(detectedModules.begin()), std::make_move_iterator(detectedModules.end()));

    RELEASE(m_syncRoot);

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
