#pragma once

#include <cstddef>
#include <string_view>

// Decoder for application/x-www-form-urlencoded bodies (e.g. "a=1&b=hello+world%21")
class FormDecoder final
{
public:
    FormDecoder() = delete; // Prevent instantiation of static class

    // Looks up `key` in `body` and writes its URL-decoded, NUL-terminated value to `out`.
    // Returns false when the key is missing, the decoded value does not fit in `out`,
    // or the value contains an encoded NUL byte. `out` is left empty on failure.
    static bool TryGetValue(std::string_view body, std::string_view key, char* out, size_t outSize) noexcept;

private:
    static bool TryDecode(std::string_view encoded, char* out, size_t outSize) noexcept;
};
