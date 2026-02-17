#include "FilterCollection.h"

#include <IniReader.h>
#include <IniWriter.h>
#include <LockGuard.h>

FilterCollection::FilterCollection(IStorage& storage, std::string_view fileName) noexcept
    : m_storage(storage)
    , m_fileName(fileName)
{
}

bool FilterCollection::LoadFromFile(const FilterFactoryFn& factory) noexcept
{
    IniReader iniReader;

    std::string id;
    std::string typeName;
    std::string connections;

    iniReader.OnSection([&](std::string_view section)
    {
        id.clear();
        typeName.clear();
        connections.clear();
    });

    std::vector<std::unique_ptr<Filter>> filters;
    iniReader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value)
    {
        if (section != "Filter")
            return;

        if (key == "Id") id = value;
        else if (key == "Type") typeName = value;
        else if (key == "Connections") connections = value;

        if (!id.empty() && !typeName.empty() && !connections.empty())
        {
            if (auto filter = factory(id, typeName, connections))
                filters.push_back(std::move(filter));
        }
    });

    if (!m_storage.ReadFileInChunks(
        m_fileName,
        [&](const char* chunk, size_t chunkSize)
        {
            iniReader.Feed(chunk, chunkSize);
            return true;
        }))
    {
        return false;
    }

    iniReader.Finalize();

    m_filters.clear();
    m_filters.reserve(filters.size());
    for (auto& filter : filters)
    {
        const auto& id = filter->GetId();
        m_filters.emplace(id, std::shared_ptr<Filter>(std::move(filter)));
    }

    return true;
}

bool FilterCollection::Clear() noexcept
{
    m_filters.clear();
    m_storage.RemoveFile(m_fileName);
    return true;
}

std::shared_ptr<Filter> FilterCollection::AppendFilter(std::unique_ptr<Filter> filter, std::string_view connections) noexcept
{
    std::shared_ptr owned = std::move(filter);
    m_filters.emplace(owned->GetId(), owned);

    IniWriter iniWriter;
    iniWriter.WriteSection("Filter");
    iniWriter.WriteKeyValue("Id", owned->GetId());
    iniWriter.WriteKeyValue("Type", GetFilterTypeName(owned->GetType()));
    iniWriter.WriteKeyValue("Connections", connections);
    
    m_storage.AppendFile(m_fileName, iniWriter.GetContent());
    m_storage.AppendFile(m_fileName, "\n");

    return owned;
}

std::shared_ptr<Filter> FilterCollection::TryGetFilterById(std::string_view id) const noexcept
{
    auto it = m_filters.find(id);
    if (it == m_filters.end())
        return nullptr;

    return it->second;
}
