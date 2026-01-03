#pragma once

#include <string>

class IniWriter final
{
public:
    IniWriter();

    void WriteSection(const std::string_view section);
    void WriteKeyValue(const std::string_view key, const std::string_view value);
    void WriteComment(const std::string_view comment);

    const std::string& GetContent() const;

private:
    bool m_isFirstSection = true;
    std::string m_content;
};
