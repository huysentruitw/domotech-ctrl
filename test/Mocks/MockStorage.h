#include <IStorage.h>

class MockStorage final : public IStorage
{
public:
    bool Format() noexcept override
    {
        return true;
    }

    bool WriteFile(std::string_view fileName, std::string_view content) noexcept override
    {
        return true;
    }

    bool ReadFile(std::string_view fileName, char* buffer, size_t bufferSize, size_t& read) noexcept override
    {
        return true;
    }

    bool ReadFileInChunks(std::string_view fileName, const std::function<void(const char*, size_t)>& onChunk) noexcept override
    {
        return true;
    }

    bool RemoveFile(std::string_view fileName) noexcept override
    {
        return true;
    }
};
