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
    bool WriteFile(const char* path, const std::vector<uint8_t>& data) noexcept override;
    bool ReadFile(const char* path, std::vector<uint8_t>& out) noexcept override;
    bool RemoveFile(const char* path) noexcept override;

private:
    const Lock m_syncRoot;
    const esp_vfs_littlefs_conf_t m_config;
};

#endif