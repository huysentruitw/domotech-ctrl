#pragma once

#include <cstdint>
#include <algorithm>

struct DimmerControlValue
{
public:
    constexpr DimmerControlValue(uint8_t percentage = 0, uint8_t fadeTimeInSeconds = 0) noexcept
    {
        m_percentage = std::clamp((int)percentage, 0, 100);
        m_fadeTimeInSeconds = std::clamp((int)fadeTimeInSeconds, 0, 99);
    }

    constexpr bool operator==(const DimmerControlValue& other) const noexcept = default;

    constexpr uint8_t GetPercentage() const noexcept
    {
        return m_percentage;
    }

    constexpr uint8_t GetFadeTimeInSeconds() const noexcept
    {
        return m_fadeTimeInSeconds;
    }

    constexpr operator int() const noexcept
    {
        return m_percentage;
    }

private:
    uint8_t m_percentage;
    uint8_t m_fadeTimeInSeconds;
};
