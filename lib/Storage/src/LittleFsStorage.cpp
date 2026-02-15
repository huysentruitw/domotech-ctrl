#ifndef NATIVE_BUILD

#include "LittleFsStorage.h"

#include <LockGuard.h>

#include "esp_log.h"

#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>

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

bool LittleFsStorage::WriteFile(std::string_view fileName, std::string_view content) noexcept
{
    return InternalWriteFile(fileName, content, false);
}

bool LittleFsStorage::AppendFile(std::string_view fileName, std::string_view content) noexcept
{
    return InternalWriteFile(fileName, content, true);
}

bool LittleFsStorage::InternalWriteFile(std::string_view fileName, std::string_view content, bool append) noexcept
{
    LockGuard guard(m_syncRoot);

    char path[64];
    if (!MakeFullPath(fileName, path, sizeof(path)))
    {
        ESP_LOGE(TAG, "Invalid or too-long file name");
        return false;
    }

    FILE* f = fopen(path, append ? "ab" : "wb");
    if (!f)
    {
        ESP_LOGE(TAG, "Failed to open %s for %s", path, append ? "appending" : "writing");
        return false;
    }

    const size_t written = fwrite(content.data(), 1, content.size(), f);
    fclose(f);

    return written == content.size();
}

bool LittleFsStorage::ReadFile(std::string_view fileName, char* buffer, size_t bufferSize, size_t& read) noexcept
{
    LockGuard guard(m_syncRoot);

    read = 0;

    char path[64];
    if (!MakeFullPath(fileName, path, sizeof(path)))
    {
        ESP_LOGE(TAG, "Invalid or too-long file name");
        return false;
    }

    FILE* f = fopen(path, "rb");
    if (!f)
    {
        ESP_LOGE(TAG, "Failed to open %s for reading", path);
        return false;
    }

    fseek(f, 0, SEEK_END);
    const long ret = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (ret < 0)
    {
        fclose(f);
        return false;
    }

    const size_t fileSize = static_cast<size_t>(ret);

    if (fileSize > bufferSize)
    {
        ESP_LOGE(TAG, "Buffer of size %d too small to read %s of size %d", bufferSize, path, fileSize);
        fclose(f);
        return false;
    }

    read = fread(buffer, 1, fileSize, f);
    fclose(f);

    return read == fileSize;
}

bool LittleFsStorage::ReadFileInChunks(std::string_view fileName, const std::function<bool(const char*, size_t)>& onChunk) noexcept
{
    LockGuard guard(m_syncRoot);

    char path[64];
    if (!MakeFullPath(fileName, path, sizeof(path)))
    {
        ESP_LOGE(TAG, "Invalid or too-long file name");
        return false;
    }

    FILE* f = fopen(path, "rb");
    if (!f)
    {
        ESP_LOGE(TAG, "Failed to open %s for reading", path);
        return false;
    }

    char buffer[256];

    while (true)
    {
        size_t n = fread(buffer, 1, sizeof(buffer), f);
        if (n > 0)
        {
            if (!onChunk(buffer, n))
            {
                fclose(f);
                return false; // aborted by callback
            }
        }

        if (n < sizeof(buffer))
            break; // EOF or error
    }

    fclose(f);

    return true;
}

bool LittleFsStorage::RemoveFile(std::string_view fileName) noexcept
{
    LockGuard guard(m_syncRoot);

    char path[64];
    if (!MakeFullPath(fileName, path, sizeof(path)))
    {
        ESP_LOGE(TAG, "Invalid or too-long file name");
        return false;
    }

    if (unlink(path) == 0)
        return true;

    const int err = errno;

    if (err == ENOENT)
    {
        ESP_LOGW(TAG, "Trying to remove %s but file was not found", path);
        return false;
    }

    ESP_LOGE(TAG, "Failed to remove %s (errno=%d)", path, err);
    return false;
}

bool LittleFsStorage::EnumerateFiles(const std::function<bool(std::string_view)>& onFile) const noexcept
{
    LockGuard guard(m_syncRoot);

    DIR* dir = opendir(m_config.base_path);
    if (!dir)
        return false;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_type == DT_REG)
        {
            if (!onFile(entry->d_name))
            {
                closedir(dir);
                return false; // aborted by callback 
            }
        }
    }

    closedir(dir);
    return true;
}

bool LittleFsStorage::MakeFullPath(std::string_view fileName, char* out, size_t outSize) const noexcept
{
    if (fileName.empty() || fileName.find('/') != std::string_view::npos)
        return false;

    int written = snprintf(out, outSize, "%s/%.*s", m_config.base_path, (int)fileName.size(), fileName.data());
    return written > 0 && static_cast<size_t>(written) < outSize;
}

#endif
