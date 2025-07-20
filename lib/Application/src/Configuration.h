#pragma once

#include <memory>
#include <string>
#include <vector>

#include <FilterBase.h>
#include <ModuleBase.h>

class Configuration final
{
public:
    void Clear();

    void AddFilter(std::shared_ptr<FilterBase> filter);
    void AddModules(std::vector<std::shared_ptr<ModuleBase>> modules);

    std::string ToString() const;

private:
    std::vector<std::shared_ptr<FilterBase>> m_filters;
    std::vector<std::shared_ptr<ModuleBase>> m_modules;
};
