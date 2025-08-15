#include "IniReader.h"

IniReader::IniReader(const std::string& content)
{
    m_stream.str(content);
}

void IniReader::Process(
    const std::function<void(const std::string& /*sectionName*/)>& sectionHandler,
    const std::function<void(const std::string& /*sectionName*/, const std::string& /*key*/, const std::string& /*value*/)>& keyValueHandler)
{
    std::string line;
    std::string currentSection;

    while (std::getline(m_stream, line)) {
        TrimInPlace(line);

        if (line.empty() || line[0] == ';') {
            continue; // Skip empty lines and comments
        }

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            TrimInPlace(currentSection);
            sectionHandler(currentSection);
        } else {
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                TrimInPlace(key);
                TrimInPlace(value);
                keyValueHandler(currentSection, key, value);
            }
        }
    }
}

void IniReader::TrimInPlace(std::string& str)
{
    auto start = str.find_first_not_of(" \t");
    auto end = str.find_last_not_of(" \t");
    
    if (start == std::string::npos) {
        str.clear();
        return;
    }

    str = str.substr(start, end - start + 1);
}
