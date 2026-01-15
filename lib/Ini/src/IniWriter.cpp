#include "IniWriter.h"

IniWriter::IniWriter() noexcept
{
    // Reserve some space to avoid reallocations
    m_content.reserve(1024);
}

void IniWriter::WriteSection(std::string_view section) noexcept
{
    if (!m_isFirstSection)
        m_content.append("\n");

    m_content.append("[");
    m_content.append(section);
    m_content.append("]\n");
    m_isFirstSection = false;
}

void IniWriter::WriteKeyValue(std::string_view key, std::string_view value) noexcept
{
    m_content.append(key);
    m_content.append("=");
    m_content.append(value);
    m_content.append("\n");
}

void IniWriter::WriteComment(std::string_view comment) noexcept
{
    m_content.append("; ");
    m_content.append(comment);
    m_content.append("\n");
}

const std::string& IniWriter::GetContent() const noexcept
{
    return m_content;
}
