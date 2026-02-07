#pragma once

#include <cstdint>
#include <vector>

class IStorage
{
public:
    virtual ~IStorage() = default;

    virtual bool Format() noexcept = 0;
    virtual bool WriteFile(const char* path, const std::vector<uint8_t>& data) noexcept = 0;
    virtual bool ReadFile(const char* path, std::vector<uint8_t>& out) noexcept = 0;
    virtual bool RemoveFile(const char* path) noexcept = 0;
};
