#include <IStorage.h>

class MockStorage final : public IStorage
{
public:
    bool Format() noexcept override
    {
        return true;
    }

    bool WriteFile(std::string_view fileName, const std::vector<uint8_t>& data) noexcept override
    {
        return true;
    }

    bool ReadFile(std::string_view fileName, std::vector<uint8_t>& out) noexcept override
    {
        return true;
    }

    bool ReadFileInChunks(std::string_view fileName, const std::function<void(const uint8_t*, size_t)>& onChunk) noexcept override
    {
        return true;
    }

    bool RemoveFile(std::string_view fileName) noexcept override
    {
        return true;
    }
};
