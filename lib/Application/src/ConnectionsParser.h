#pragma once

/*
* Example string: Connections=I0=A13:O3,O0=A12:I1,O1=A13:I3
*
* Grammar
* Connections  := Mapping { "," Mapping }
* Mapping      := LocalPin "=" RemoteModule ":" RemotePin
* LocalPin     := ("I" | "O") Digit+
* RemoteModule := "A" Digit+
* RemotePin    := ("I" | "O") Digit+
*/

#include <Pin.h>
#include <StaticList.h>

#include <cstddef>
#include <optional>
#include <string_view>

struct PinIdentifier
{
    PinDirection Direction;
    uint8_t Index;
};

struct ModuleIdentifier
{
    uint8_t Address;
};

struct Mapping
{
    PinIdentifier LocalPin;        // "I0" -> { Input, Index: 0}
    ModuleIdentifier RemoteModule; // "A13" -> { Address: 13 }
    PinIdentifier RemotePin;       // "O3" -> { Output, Index: 3 }
};

constexpr bool IsDigit(char c) noexcept
{
    return (c >= '0' && c <= '9');
}

constexpr void SkipWhitespace(char const *&cur, char const *end) noexcept
{
    while (cur < end && (*cur == ' ' || *cur == '\t'))
        ++cur;
}

constexpr bool MatchChar(char expected, char const *&cur, char const *end) noexcept
{
    if (cur >= end || *cur != expected)
        return false;

    ++cur;
    return true;
}

// Local or remote pin: "I0", "O3", etc.
constexpr bool ParsePin(PinIdentifier &out, char const *&cur, char const *end) noexcept
{
    if (cur >= end || (*cur != 'I' && *cur != 'O'))
        return false;
    
    out.Direction = *cur == 'I' ? PinDirection::Input : PinDirection::Output;

    ++cur;

    if (cur >= end || !IsDigit(*cur))
        return false;

    uint32_t value = 0;
    while (cur < end && IsDigit(*cur))
    {
        value = value * 10 + (*cur - '0');
        if (value > 255)
            return false; // enforce uint8_t range

        ++cur;
    }

    out.Index = static_cast<uint8_t>(value);
    return true;
}

// Module: "A13"
constexpr bool ParseModule(ModuleIdentifier &out, char const *&cur, char const *end) noexcept
{
    if (cur >= end || *cur != 'A')
        return false;

    ++cur;

    if (cur >= end || !IsDigit(*cur))
        return false;

    uint32_t value = 0;
    while (cur < end && IsDigit(*cur))
    {
        value = value * 10 + (*cur - '0');
        if (value > 255) return false; // enforce uint8_t range
        ++cur;
    }

    out.Address = static_cast<uint8_t>(value);
    return true;
}

// Mapping: "I0=A13:O3"
constexpr bool ParseMapping(Mapping &out, char const *&cur, char const *end) noexcept
{
    SkipWhitespace(cur, end);

    PinIdentifier localPin;
    ModuleIdentifier remoteModule;
    PinIdentifier remotePin;

    if (!ParsePin(localPin, cur, end)) return false;
    if (!MatchChar('=', cur, end)) return false;
    if (!ParseModule(remoteModule, cur, end)) return false;
    if (!MatchChar(':', cur, end)) return false;
    if (!ParsePin(remotePin, cur, end)) return false;

    out.LocalPin = localPin;
    out.RemoteModule = remoteModule;
    out.RemotePin = remotePin;
    return true;
}

template<std::size_t MaxMappings>
constexpr std::optional<StaticList<Mapping, MaxMappings>> TryParseConnections(std::string_view text) noexcept
{
    char const *cur = text.data();
    char const *end = text.data() + text.size();

    if (cur == end)
        return StaticList<Mapping, MaxMappings>{};

    StaticList<Mapping, MaxMappings> result;

    // First mapping (required if non-empty)
    {
        Mapping m;
        if (!ParseMapping(m, cur, end))
            return std::nullopt;

        if (!result.Add(m))
            return std::nullopt;
    }

    // Subsequent mappings: ,Mapping
    while (cur < end) {
        SkipWhitespace(cur, end);
        if (cur >= end)
            break;

        if (!MatchChar(',', cur, end))
            return std::nullopt;

        SkipWhitespace(cur, end);
        if (cur >= end)
            return std::nullopt; // Trailing comma is invalid

        Mapping m;
        if (!ParseMapping(m, cur, end))
            return std::nullopt;

        if (!result.Add(m))
            return std::nullopt;
    }

    return result;
}
