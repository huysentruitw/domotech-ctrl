#pragma once

struct DigitalValue
{
public:
    explicit DigitalValue(bool value = false) noexcept : m_value(value) {}

    bool operator==(const DigitalValue& other) const noexcept = default;

    operator bool() const noexcept
    {
        return m_value;
    }

private:    
    bool m_value;
};
