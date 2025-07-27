#include "Configuration.h"
#include "KnownModuleIdentifiers.h"

#include <sstream>

void Configuration::Clear()
{
    m_filters.clear();
    m_modules.clear();
}

void Configuration::AddFilter(std::shared_ptr<Filter> filter)
{
    m_filters.push_back(std::move(filter));
}

void Configuration::AddModules(std::vector<std::shared_ptr<Module>> modules)
{
    m_modules.insert(m_modules.end(), std::make_move_iterator(modules.begin()), std::make_move_iterator(modules.end()));
}

std::string Configuration::ToString() const
{   
    std::ostringstream result;

    result << "[Modules]" << std::endl;

    for (const auto& module : m_modules)
    {
        result << std::to_string(module->GetAddress()) << "=";

        switch (module->GetType())
        {
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
    for (const auto& filter : m_filters)
    {
        result << "FLT" << std::endl;
    }

    return result.str();
}
