#pragma once

#include <string>

class IniWriter final
{
public:
    IniWriter();

    void WriteSection(std::string_view section);
    void WriteKeyValue(std::string_view key, std::string_view value);
    void WriteComment(std::string_view comment);

    const std::string& GetContent() const;

private:
    bool m_isFirstSection = true;
    std::string m_content;
};
