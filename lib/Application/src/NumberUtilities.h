#pragma once

#include <cstdint>
#include <optional>
#include <string>

class NumberUtilities final
{
public:
    NumberUtilities() = delete; // Prevent instantiation of static class
    
    static std::string ToHex4(uint16_t value) noexcept;
    static std::optional<int> ParseInt(std::string_view sv, int base = 10) noexcept;
};
