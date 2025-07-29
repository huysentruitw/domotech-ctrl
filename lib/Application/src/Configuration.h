#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Filter.h>
#include <Module.h>

class Configuration final
{
public:
    std::string GetKnownFiltersIni() const;

    void Clear();

    void AddFilter(std::shared_ptr<Filter> filter);
    void AddModules(std::vector<std::shared_ptr<Module>> modules);

    std::string ToString() const;

private:
    std::vector<std::shared_ptr<Filter>> m_filters;
    std::vector<std::shared_ptr<Module>> m_modules;
};
