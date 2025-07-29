#pragma once

#include <cstdint>
#include <algorithm>

struct DimmerControlValue
{
public:
    DimmerControlValue(int percentage = 0, int fadeTimeInSeconds = 0)
    {
        m_percentage = std::clamp(percentage, 0, 100);
        m_fadeTimeInSeconds = std::clamp(fadeTimeInSeconds, 0, 99);
    }

    bool operator==(const DimmerControlValue& other) const = default;

    int GetPercentage() const {
        return m_percentage;
    }

    int GetFadeTimeInSeconds() const {
        return m_fadeTimeInSeconds;
    }

    operator int() const {
        return m_percentage;
    }

private:
    uint8_t m_percentage;
    uint8_t m_fadeTimeInSeconds;
};
