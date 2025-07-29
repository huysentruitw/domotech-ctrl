#pragma once

#include <sstream>
#include <string>

class IniWriter final
{
public:
    void WriteSection(const std::string& section);
    void WriteKeyValue(const std::string& key, const std::string& value);
    void WriteComment(const std::string& comment);

    const std::string GetContent() const;

private:
    bool m_isFirstSection = true;
    std::ostringstream m_stream;
};
