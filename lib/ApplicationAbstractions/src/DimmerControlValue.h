#pragma once

#include <cstdint>
#include <algorithm>

struct DimmerControlValue
{
public:
    DimmerControlValue(int percentage = 0, int fadeTimeInSeconds = 0) noexcept
    {
        m_percentage = std::clamp(percentage, 0, 100);
        m_fadeTimeInSeconds = std::clamp(fadeTimeInSeconds, 0, 99);
    }

    bool operator==(const DimmerControlValue& other) const noexcept = default;

    int GetPercentage() const noexcept
    {
        return m_percentage;
    }

    int GetFadeTimeInSeconds() const noexcept
    {
        return m_fadeTimeInSeconds;
    }

    operator int() const noexcept
    {
        return m_percentage;
    }

private:
    uint8_t m_percentage;
    uint8_t m_fadeTimeInSeconds;
};
