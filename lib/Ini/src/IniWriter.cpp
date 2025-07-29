#include "IniWriter.h"

void IniWriter::WriteSection(const std::string& section)
{
    if (!m_isFirstSection) {
        m_stream << "\n";
    }

    m_stream << "[" << section << "]\n";
    m_isFirstSection = false;
}

void IniWriter::WriteKeyValue(const std::string& key, const std::string& value)
{
    m_stream << key << "=" << value << "\n";
}

void IniWriter::WriteComment(const std::string& comment)
{
    m_stream << "; " << comment << "\n";
}

const std::string IniWriter::GetContent() const
{
    return m_stream.str();
}
