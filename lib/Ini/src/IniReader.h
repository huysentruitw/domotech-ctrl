#pragma once

#include <functional>
#include <string>

class IniReader final
{
public:
    IniReader() noexcept = default;

    void Feed(const char* data, size_t len) noexcept;

    void Finalize() noexcept;

    void OnSection(const std::function<void(std::string_view /*section*/)>& handler) noexcept
    {
        m_sectionHandler = handler;
    }

    void OnKeyValue(const std::function<void(std::string_view /*section*/, std::string_view /*key*/, std::string_view /*value*/)>& handler) noexcept
    {
        m_keyValueHandler = handler;
    }

private:
    void ProcessLine(std::string_view line) noexcept;
    static std::string_view Trim(std::string_view s) noexcept;

    std::string m_buffer;
    std::string m_currentSection;

    std::function<void(std::string_view)> m_sectionHandler;
    std::function<void(std::string_view, std::string_view, std::string_view)> m_keyValueHandler;
};
