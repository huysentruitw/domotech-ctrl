#include "IdSanitizer.h"

std::string IdSanitizer::Sanitize(std::string_view id, size_t maxLength) noexcept
{
    std::string result;
    result.reserve(std::min((size_t)id.size(), maxLength));

    for (char c : id) {
        if (result.size() == maxLength)
            break;

        if (c == ' ')
        {
            result.push_back('-');
            continue;
        }

        char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if ((lower >= 'a' && lower <= 'z') || (lower >= '0' && lower <= '9') || lower == '_' || lower == '-') {
            result.push_back(lower);
        }
    }

    return result;
}
