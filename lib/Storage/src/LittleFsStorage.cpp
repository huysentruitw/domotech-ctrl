#ifndef NATIVE_BUILD

#include "LittleFsStorage.h"

#include <LockGuard.h>

#include "esp_log.h"

#include <stdio.h>
#include <unistd.h>

#define TAG "STORAGE"

LittleFsStorage::LittleFsStorage(const char* partitionLabel) noexcept
    : m_syncRoot(false)
    , m_config
    {
        .base_path = "/littlefs",
        .partition_label = partitionLabel,
        .partition = nullptr,
        .format_if_mount_failed = true,
        .read_only = false,
        .dont_mount = false,
        .grow_on_mount = false,
    }
{
    LockGuard guard(m_syncRoot);

    esp_err_t ret = esp_vfs_littlefs_register(&m_config);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        else if (ret == ESP_ERR_NOT_FOUND)
            ESP_LOGE(TAG, "Failed to find partition '%s'", m_config.partition_label);
        else
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));

        return;
    }

    size_t totalBytes = 0, usedBytes = 0;
    ret = esp_littlefs_info(m_config.partition_label, &totalBytes, &usedBytes);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get partition information (%s)", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "Partition size (in bytes), total: %d, used: %d", totalBytes, usedBytes);
}

bool LittleFsStorage::Format() noexcept
{
    LockGuard guard(m_syncRoot);

    esp_vfs_littlefs_unregister(m_config.partition_label);
    
    esp_err_t ret = esp_littlefs_format(m_config.partition_label);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Format failed (%s)", esp_err_to_name(ret));
        return false;
    }

    ret = esp_vfs_littlefs_register(&m_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Mount after format failed (%s)", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "Format successful");
    return true;
}

bool LittleFsStorage::WriteFile(const char* path, const std::vector<uint8_t>& data) noexcept
{
    LockGuard guard(m_syncRoot);

    FILE* f = fopen(path, "wb");
    if (!f)
    {
        ESP_LOGE(TAG, "Failed to open %s for writing", path);
        return false;
    }

    const size_t written = fwrite(data.data(), 1, data.size(), f);
    fclose(f);

    return written == data.size();
}

bool LittleFsStorage::ReadFile(const char* path, std::vector<uint8_t>& out) noexcept
{
    LockGuard guard(m_syncRoot);

    FILE* f = fopen(path, "rb");
    if (!f)
    {
        ESP_LOGE(TAG, "Failed to open %s for reading", path);
        return false;
    }

    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0)
    {
        fclose(f);
        return false;
    }

    out.resize(size);
    const size_t read = fread(out.data(), 1, size, f);
    fclose(f);

    return read == static_cast<size_t>(size);
}

bool LittleFsStorage::ReadFileInChunks(const char* path, const std::function<void(const uint8_t*, size_t)>& onChunk) noexcept
{
    LockGuard guard(m_syncRoot);

    FILE* f = fopen(path, "rb");
    if (!f)
    {
        ESP_LOGE(TAG, "Failed to open %s for reading", path);
        return false;
    }

    uint8_t buffer[256];

    while (true)
    {
        size_t n = fread(buffer, 1, sizeof(buffer), f);
        if (n > 0)
            onChunk(buffer, n);

        if (n < sizeof(buffer))
            break; // EOF or error
    }

    fclose(f);

    return true;
}

bool LittleFsStorage::RemoveFile(const char* path) noexcept
{
    LockGuard guard(m_syncRoot);
    return unlink(path);
}

#endif
