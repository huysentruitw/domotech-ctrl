#include "FilterCollection.h"

FilterCollection::FilterCollection() noexcept
{
}

FilterCollection FilterCollection::LoadFromFile(IStorage& storage, std::string_view fileName) noexcept
{
    // TODO Implement
    return FilterCollection{};
}

void FilterCollection::SaveToFile(IStorage& storage, std::string_view fileName) const noexcept
{
    // TODO Implement
}

std::shared_ptr<Filter> FilterCollection::TryGetFilterById(std::string_view id) const noexcept
{
    auto it = m_filters.find(id);
    if (it == m_filters.end())
        return nullptr;

    return it->second;
}
