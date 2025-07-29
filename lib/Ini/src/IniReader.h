#pragma once

#include <functional>
#include <sstream>
#include <string>

class IniReader final
{
public:
    IniReader(const std::string& content);

    void Process(
        const std::function<void(const std::string& /*sectionName*/)>& sectionHandler,
        const std::function<void(const std::string& /*sectionName*/, const std::string& /*key*/, const std::string& /*value*/)>& keyValueHandler);

private:
    bool m_isFirstSection = true;
    std::istringstream m_stream;
};
