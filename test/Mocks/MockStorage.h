#include <IStorage.h>

class MockStorage final : public IStorage
{
public:
    bool Format() noexcept override
    {
        return true;
    }

    bool WriteFile(const char* path, const std::vector<uint8_t>& data) noexcept override
    {
        return true;
    }

    bool ReadFile(const char* path, std::vector<uint8_t>& out) noexcept override
    {
        return true;
    }

    bool RemoveFile(const char* path) noexcept override
    {
        return true;
    }
};
