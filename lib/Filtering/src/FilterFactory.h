#pragma once

#include "Filter.h"

#include <memory>
#include <string>

class FilterFactory final
{
public:
    FilterFactory() = delete; // Prevent instantiation of static class
    
    static std::unique_ptr<Filter> TryCreateFilterByTypeName(const std::string_view filterName);
};
