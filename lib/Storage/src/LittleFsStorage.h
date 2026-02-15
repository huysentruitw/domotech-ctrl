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
    bool WriteFile(std::string_view fileName, std::string_view content) noexcept override;
    bool AppendFile(std::string_view fileName, std::string_view content) noexcept override;
    bool ReadFile(std::string_view fileName, char* buffer, size_t bufferSize, size_t& read) noexcept override;
    bool ReadFileInChunks(std::string_view fileName, const std::function<bool(const char*, size_t)>& onChunk) noexcept override;
    bool RemoveFile(std::string_view fileName) noexcept override;
    bool EnumerateFiles(const std::function<bool(std::string_view)>& onFile) const noexcept override;

private:
    bool InternalWriteFile(std::string_view fileName, std::string_view content, bool append) noexcept;

private:
    const Lock m_syncRoot;
    const esp_vfs_littlefs_conf_t m_config;

    bool MakeFullPath(std::string_view fileName, char* out, size_t outSize) const noexcept;
};

#endif
