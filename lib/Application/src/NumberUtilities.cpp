#include "NumberUtilities.h"

std::string NumberUtilities::ToHex4(uint16_t value) noexcept
{
    static constexpr char lut[] = "0123456789ABCDEF";

    char buf[5];
    buf[0] = lut[(value >> 12) & 0xF];
    buf[1] = lut[(value >> 8)  & 0xF];
    buf[2] = lut[(value >> 4)  & 0xF];
    buf[3] = lut[value & 0xF];
    buf[4] = '\0';

    return std::string(buf);
}

std::optional<int> NumberUtilities::ParseInt(std::string_view sv, int base) noexcept
{
    if (sv.empty())
        return std::nullopt;

    int sign = 1;
    size_t i = 0;

    // Optional sign
    if (sv[0] == '-') {
        sign = -1;
        i = 1;
    } else if (sv[0] == '+') {
        i = 1;
    }

    if (i == sv.size())
        return std::nullopt; // only '+' or '-'

    int value = 0;

    for (; i < sv.size(); ++i)
    {
        char c = sv[i];
        int digit;

        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'A' && c <= 'F')
            digit = 10 + (c - 'A');
        else if (c >= 'a' && c <= 'f')
            digit = 10 + (c - 'a');
        else
            return std::nullopt; // invalid character

        if (digit >= base)
            return std::nullopt;

        // Overflow‑safe accumulation
        int next = value * base + digit;
        if (next < value)
            return std::nullopt; // overflow

        value = next;
    }

    return value * sign;
}
