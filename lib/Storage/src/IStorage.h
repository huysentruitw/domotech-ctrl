#pragma once

#include <functional>
#include <string_view>

class IStorage
{
public:
    virtual ~IStorage() = default;

    virtual bool Format() noexcept = 0;
    virtual bool WriteFile(std::string_view fileName, std::string_view content) noexcept = 0;
    virtual bool ReadFile(std::string_view fileName, char* buffer, size_t bufferSize, size_t& read) noexcept = 0;
    virtual bool ReadFileInChunks(std::string_view fileName, const std::function<void(const char*, size_t)>& onChunk) noexcept = 0;
    virtual bool RemoveFile(std::string_view fileName) noexcept = 0;
};
