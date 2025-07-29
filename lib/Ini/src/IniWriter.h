#pragma once

#include <sstream>
#include <string>

class IniWriter final
{
public:
    void WriteSection(const std::string& section)
    {
        if (!m_isFirstSection) {
            m_stream << "\n";
        }

        m_stream << "[" << section << "]\n";
        m_isFirstSection = false;
    }

    void WriteKeyValue(const std::string& key, const std::string& value)
    {
        m_stream << key << " = " << value << "\n";
    }

    void WriteComment(const std::string& comment)
    {
        m_stream << "; " << comment << "\n";
    }

    const std::string getContent() const
    {
        return m_stream.str();
    }

private:
    bool m_isFirstSection = true;
    std::ostringstream m_stream;
};
