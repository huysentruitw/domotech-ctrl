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
private:
    struct FilterEntry { std::shared_ptr<Filter> filter; std::string connections; };

    using FilterMap = std::unordered_map<std::string, FilterEntry, StringHash, std::equal_to<>>;

    class FilterIterator final
    {
    public:
        explicit FilterIterator(FilterMap::const_iterator it) : m_it(it) { }
        const std::shared_ptr<Filter>& operator*() const noexcept { return m_it->second.filter; }
        const std::shared_ptr<Filter>* operator->() const noexcept { return &m_it->second.filter; }
        FilterIterator& operator++() noexcept { ++m_it; return *this; }
        bool operator==(const FilterIterator& other) const noexcept { return m_it == other.m_it; }
        bool operator!=(const FilterIterator& other) const noexcept { return m_it != other.m_it; }

    private:
        FilterMap::const_iterator m_it;
    };

public:
    FilterCollection(IStorage& storage, std::string_view fileName) noexcept;

    bool LoadFromFile(const FilterFactoryFn& factory) noexcept;
    bool Clear() noexcept;

    std::shared_ptr<Filter> AddFilter(std::unique_ptr<Filter> filter, std::string_view connections) noexcept;
    bool RemoveFilter(std::string_view id) noexcept;

    std::shared_ptr<Filter> TryGetFilterById(std::string_view id) const noexcept;

    auto begin() const noexcept { return FilterIterator { m_filters.begin() }; }
    auto end() const noexcept { return FilterIterator { m_filters.end() }; }

private:
    IStorage& m_storage;
    std::string m_fileName;

    FilterMap m_filters;

    bool SaveToFile() const noexcept;

    bool AppendFilterToFile(std::string_view fileName, const Filter& filter, std::string_view connections) const noexcept;
};
