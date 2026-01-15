#include "IniReader.h"

void IniReader::Feed(const char* data, size_t len) noexcept
{
    for (size_t i = 0; i < len; i++)
    {
        char c = data[i];

        if (c == '\n')
        {
            ProcessLine(m_buffer);
            m_buffer.clear();
        }
        else if (c != '\r')
        {
            m_buffer.push_back(c);
        }
    }
}

void IniReader::Finalize() noexcept
{
    if (!m_buffer.empty())
    {
        ProcessLine(m_buffer);
        m_buffer.clear();
    }
}

void IniReader::ProcessLine(std::string_view line) noexcept
{
    line = Trim(line);
    if (line.empty() || line[0] == ';')
        return; // Skip empty lines and comments

    if (line.front() == '[' && line.back() == ']')
    {
        auto section = Trim(line.substr(1, line.size() - 2));
        m_currentSection.assign(section);
        if (m_sectionHandler)
            m_sectionHandler(section);
        return;
    }

    size_t pos = line.find('=');
    if (pos != std::string_view::npos)
    {
        std::string_view key = Trim(line.substr(0, pos));
        std::string_view value = Trim(line.substr(pos + 1));
        if (m_keyValueHandler)
            m_keyValueHandler(m_currentSection, key, value);
    }
}

std::string_view IniReader::Trim(std::string_view s) noexcept
{
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t'))
        start++;

    size_t end = s.size();
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t'))
        end--;
        
    return s.substr(start, end - start);
}
