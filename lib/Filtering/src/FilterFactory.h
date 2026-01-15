#pragma once

#include "Filter.h"

#include <memory>
#include <string>

class FilterFactory final
{
public:
    FilterFactory() = delete; // Prevent instantiation of static class
    
    static std::unique_ptr<Filter> TryCreateFilterByTypeName(std::string_view typeName, std::string_view id) noexcept;
};
