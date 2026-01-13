#pragma once

#include <cstddef>
#include <string>

class IdSanitizer final
{
public:
    IdSanitizer() = delete; // Prevent instantiation of static class

    static std::string Sanitize(std::string_view id, size_t maxLength = 32) noexcept;
};
