#pragma once

#include <cstdint>
#include <functional>
#include <vector>

class IStorage
{
public:
    virtual ~IStorage() = default;

    virtual bool Format() noexcept = 0;
    virtual bool WriteFile(const char* path, const std::vector<uint8_t>& data) noexcept = 0;
    virtual bool ReadFile(const char* path, std::vector<uint8_t>& out) noexcept = 0;
    virtual bool ReadFileInChunks(const char* path, const std::function<void(const uint8_t*, size_t)>& onChunk) noexcept = 0;
    virtual bool RemoveFile(const char* path) noexcept = 0;
};
