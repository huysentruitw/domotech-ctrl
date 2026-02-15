#pragma once

#include <Filter.h>
#include <IStorage.h>
#include <StringHash.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

using FilterFactoryFn = std::function<std::unique_ptr<Filter>(std::string_view id, std::string_view typeName, std::string_view connections)>;

class FilterCollection final
{
public:
    FilterCollection(IStorage& storage, std::string_view fileName) noexcept;

    bool LoadFromFile(const FilterFactoryFn& factory) noexcept;
    bool Clear() noexcept;
    std::shared_ptr<Filter> AppendFilter(std::unique_ptr<Filter> filter, std::string_view connections) noexcept;

    std::shared_ptr<Filter> TryGetFilterById(std::string_view id) const noexcept;

    auto begin() const noexcept { return m_filters.begin(); }
    auto end() const noexcept { return m_filters.end(); }

private:
    IStorage& m_storage;
    std::string m_fileName;
    std::unordered_map<std::string, std::shared_ptr<Filter>, StringHash, std::equal_to<>> m_filters;
};
