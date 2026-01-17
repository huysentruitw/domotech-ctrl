#pragma once

struct TemperatureValue
{
public:    
    constexpr explicit TemperatureValue(float value = 0.0f) noexcept : m_value(value) {}

    constexpr bool operator==(const TemperatureValue&) const noexcept = default;

    constexpr explicit operator float() const noexcept
    {
        return m_value;
    }

private:    
    float m_value;
};
