#pragma once

struct DigitalValue
{
public:    
    explicit DigitalValue(bool value) : m_value(value) {}

    bool operator==(const DigitalValue& other) const = default;

    operator bool() const {
        return m_value;
    }

private:    
    bool m_value;
};
