#pragma once

#include <IStorage.h>

#include <algorithm>
#include <cstring>
#include <map>
#include <string>

class MockStorage final : public IStorage
{
public:
    bool Format() noexcept override
    {
        m_testFiles.clear();
        return true;
    }

    bool WriteFile(std::string_view fileName, std::string_view content) noexcept override
    {
        m_testFiles[std::string(fileName)] = std::string(content);
        return true;
    }

    bool AppendFile(std::string_view fileName, std::string_view content) noexcept override
    {
        std::string key(fileName);
        auto it = m_testFiles.find(key);
        if (it != m_testFiles.end())
        {
            it->second += content;
        }
        else
        {
            m_testFiles[key] = std::string(content);
        }

        return true;
    }

    bool ReadFile(std::string_view fileName, char* buffer, size_t bufferSize, size_t& read) noexcept override
    {
        auto it = m_testFiles.find(std::string(fileName));
        if (it == m_testFiles.end())
        {
            read = 0;
            return false;
        }

        const auto& content = it->second;
        read = std::min(bufferSize, content.size());
        if (read > 0)
        {
            std::memcpy(buffer, content.data(), read);
        }

        return true;
    }

    bool ReadFileInChunks(std::string_view fileName, const std::function<bool(const char*, size_t)>& onChunk) noexcept override
    {
        auto it = m_testFiles.find(std::string(fileName));
        if (it == m_testFiles.end())
            return false;

        const auto& content = it->second;
        if (content.empty())
            return true;

        size_t mid = content.size() / 2;
        
        // First chunk
        if (!onChunk(content.data(), mid))
            return false;
        
        // Second chunk (or remaining)
        return onChunk(content.data() + mid, content.size() - mid);
    }

    bool RemoveFile(std::string_view fileName) noexcept override
    {
        auto it = m_testFiles.find(std::string(fileName));
        if (it == m_testFiles.end())
            return false;

        m_testFiles.erase(it);
        return true;
    }

    bool EnumerateFiles(const std::function<bool(std::string_view)>& onFile) const noexcept override
    {
        for (auto& [fileName, _] : m_testFiles)
        {
            if (!onFile(fileName))
                return false;
        }

        return true;
    }

    void AddTestFile(std::string_view fileName, std::string_view content) noexcept
    {
        m_testFiles.emplace(std::string(fileName), std::string(content));
    }

private:
    std::map<std::string, std::string> m_testFiles;
};
