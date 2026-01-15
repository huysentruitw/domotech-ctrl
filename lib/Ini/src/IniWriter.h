#pragma once

#include <string>

class IniWriter final
{
public:
    IniWriter() noexcept;

    void WriteSection(std::string_view section) noexcept;
    void WriteKeyValue(std::string_view key, std::string_view value) noexcept;
    void WriteComment(std::string_view comment) noexcept;

    const std::string& GetContent() const noexcept;

private:
    bool m_isFirstSection = true;
    std::string m_content;
};
