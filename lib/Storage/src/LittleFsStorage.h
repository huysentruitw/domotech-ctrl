#pragma once

#ifndef NATIVE_BUILD

#include "IStorage.h"

#include <Lock.h>

#include "esp_littlefs.h"

class LittleFsStorage final : public IStorage
{
public:
    explicit LittleFsStorage(const char* partitionLabel = "storage") noexcept;

    bool Format() noexcept override;
    bool WriteFile(std::string_view fileName, const std::vector<uint8_t>& data) noexcept override;
    bool ReadFile(std::string_view fileName, std::vector<uint8_t>& out) noexcept override;
    bool ReadFileInChunks(std::string_view fileName, const std::function<void(const uint8_t*, size_t)>& onChunk) noexcept override;
    bool RemoveFile(std::string_view fileName) noexcept override;

private:
    const Lock m_syncRoot;
    const esp_vfs_littlefs_conf_t m_config;

    bool MakeFullPath(std::string_view fileName, char* out, size_t outSize) const noexcept;
};

#endif