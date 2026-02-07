#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

class IStorage
{
public:
    virtual ~IStorage() = default;

    virtual bool Format() noexcept = 0;
    virtual bool WriteFile(std::string_view fileName, const std::vector<uint8_t>& data) noexcept = 0;
    virtual bool ReadFile(std::string_view fileName, std::vector<uint8_t>& out) noexcept = 0;
    virtual bool ReadFileInChunks(std::string_view fileName, const std::function<void(const uint8_t*, size_t)>& onChunk) noexcept = 0;
    virtual bool RemoveFile(std::string_view fileName) noexcept = 0;
};
