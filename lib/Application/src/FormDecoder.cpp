#include "FormDecoder.h"

static int HexValue(char c) noexcept
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

bool FormDecoder::TryGetValue(std::string_view body, std::string_view key, char* out, size_t outSize) noexcept
{
    if (out == nullptr || outSize == 0)
        return false;

    out[0] = '\0';

    while (!body.empty())
    {
        const size_t ampersandPos = body.find('&');
        const std::string_view pair = body.substr(0, ampersandPos);
        body = ampersandPos == std::string_view::npos ? std::string_view() : body.substr(ampersandPos + 1);

        const size_t equalsPos = pair.find('=');
        const std::string_view pairKey = pair.substr(0, equalsPos);
        if (pairKey != key)
            continue;

        const std::string_view value = equalsPos == std::string_view::npos ? std::string_view() : pair.substr(equalsPos + 1);
        return TryDecode(value, out, outSize);
    }

    return false;
}

bool FormDecoder::TryDecode(std::string_view encoded, char* out, size_t outSize) noexcept
{
    size_t length = 0;

    for (size_t i = 0; i < encoded.size(); ++i)
    {
        char c = encoded[i];

        if (c == '+')
        {
            c = ' ';
        }
        else if (c == '%' && i + 2 < encoded.size() && HexValue(encoded[i + 1]) >= 0 && HexValue(encoded[i + 2]) >= 0)
        {
            c = static_cast<char>((HexValue(encoded[i + 1]) << 4) | HexValue(encoded[i + 2]));
            i += 2;

            if (c == '\0')
            {
                out[0] = '\0';
                return false; // Embedded NUL would silently truncate the value
            }
        }

        if (length + 1 >= outSize)
        {
            out[0] = '\0';
            return false; // Does not fit (including NUL terminator)
        }

        out[length++] = c;
    }

    out[length] = '\0';
    return true;
}
