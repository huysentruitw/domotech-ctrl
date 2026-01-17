#pragma once

struct DigitalValue
{
public:
    constexpr explicit DigitalValue(bool value = false) noexcept : m_value(value) {}

    constexpr bool operator==(const DigitalValue& other) const noexcept = default;

    constexpr operator bool() const noexcept
    {
        return m_value;
    }

private:    
    bool m_value;
};
