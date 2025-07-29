#pragma once

#include <functional>
#include <sstream>
#include <string>

class IniReader final
{
public:
    IniReader(const std::string& content)
    {
        m_stream.str(content);
    }

    void Process(
        const std::function<void(const std::string&)>& sectionHandler, // Passes the name of the section
        const std::function<void(const std::string&, const std::string&, const std::string&)>& keyValueHandler) // Passes the section name, key, and value
    {
        std::string line;
        std::string currentSection;

        while (std::getline(m_stream, line)) {
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (line.empty() || line[0] == ';') {
                continue; // Skip empty lines and comments
            }

            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.size() - 2);
                sectionHandler(currentSection);
            } else {
                auto pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    keyValueHandler(currentSection, key, value);
                }
            }
        }
    }

private:
    bool m_isFirstSection = true;
    std::istringstream m_stream;
};
