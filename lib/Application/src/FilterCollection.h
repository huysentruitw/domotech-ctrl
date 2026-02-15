#pragma once

#include <Filter.h>
#include <IStorage.h>
#include <StringHash.h>

#include <memory>
#include <string>
#include <unordered_map>

class FilterCollection final
{
public:
    FilterCollection() noexcept;

    static FilterCollection LoadFromFile(IStorage& storage, std::string_view fileName) noexcept;
    void SaveToFile(IStorage& storage, std::string_view fileName) const noexcept;

    std::shared_ptr<Filter> TryGetFilterById(std::string_view id) const noexcept;

private:
    std::unordered_map<std::string, std::shared_ptr<Filter>, StringHash, std::equal_to<>> m_filters;
};
