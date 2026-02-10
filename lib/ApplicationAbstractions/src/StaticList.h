#pragma once

#include <cstddef>

template<typename T, std::size_t Max>
class StaticList
{
public:
    StaticList() = default;

    bool Add(const T& value) noexcept
    {
        if (m_count >= Max)
            return false;
        m_items[m_count++] = value;
        return true;
    }

    std::size_t Count() const noexcept { return m_count; }
    void Clear() noexcept { m_count = 0; }
    bool IsEmpty() const noexcept { return m_count == 0; }

    T& operator[](std::size_t index) noexcept { return m_items[index]; }
    const T& operator[](std::size_t index) const noexcept { return m_items[index]; }

private:
    T m_items[Max];
    std::size_t m_count = 0;
};
