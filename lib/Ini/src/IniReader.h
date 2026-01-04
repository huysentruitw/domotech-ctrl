#pragma once

#include <functional>
#include <string>

class IniReader final
{
public:
    IniReader() = default;

    void Feed(const char* data, size_t len);

    void Finalize();

    void OnSection(const std::function<void(std::string_view /*section*/)>& handler) {
        m_sectionHandler = handler;
    }

    void OnKeyValue(const std::function<void(std::string_view /*section*/, std::string_view /*key*/, std::string_view /*value*/)>& handler) {
        m_keyValueHandler = handler;
    }

private:
    void ProcessLine(std::string_view line);
    static std::string_view Trim(std::string_view s);

    std::string m_buffer;
    std::string m_currentSection;

    std::function<void(std::string_view)> m_sectionHandler;
    std::function<void(std::string_view, std::string_view, std::string_view)> m_keyValueHandler;
};
