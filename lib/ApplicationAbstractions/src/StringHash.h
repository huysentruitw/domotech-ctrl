#pragma once

#include <string>

struct StringHash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
 
    std::size_t operator()(const char* str) const noexcept        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const noexcept   { return hash_type{}(str); }
    std::size_t operator()(const std::string& str) const noexcept { return hash_type{}(str); }
};