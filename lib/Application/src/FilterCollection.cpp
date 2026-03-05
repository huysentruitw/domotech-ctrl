#include "FilterCollection.h"

#include <IniReader.h>
#include <IniWriter.h>

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

    std::vector<FilterEntry> pendingFilterEntries;

    auto finalizeFilter = [&]()
    {
        if (!id.empty() && !typeName.empty() && !connections.empty())
        {
            if (auto filter = factory(id, typeName, connections))
                pendingFilterEntries.push_back(FilterEntry { std::move(filter), std::move(connections) });
        }

        id.clear();
        typeName.clear();
        connections.clear();
    };

    iniReader.OnSection([&](std::string_view)
    {
        finalizeFilter();
    });

    iniReader.OnKeyValue([&](std::string_view section, std::string_view key, std::string_view value)
    {
        if (section != "Filter")
            return;

        if (key == "Id") id = value;
        else if (key == "Type") typeName = value;
        else if (key == "Connections") connections = value;
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
    finalizeFilter();

    m_filters.clear();
    m_filters.reserve(pendingFilterEntries.size());

    for (auto& entry : pendingFilterEntries)
        m_filters.try_emplace(entry.filter->GetId(), std::move(entry));

    return true;
}

bool FilterCollection::Clear() noexcept
{
    m_storage.RemoveFile(m_fileName); // ignore result
    m_filters.clear();
    return true;
}

std::shared_ptr<Filter> FilterCollection::AddFilter(std::unique_ptr<Filter> filter, std::string_view connections) noexcept
{
    const auto& id = filter->GetId();
    if (TryGetFilterById(id))
        return nullptr; // Filter with the same id already exists

    if (!AppendFilterToFile(m_fileName, *filter, connections))
        return nullptr; // Could not append filter to file

    std::shared_ptr owned = std::move(filter);
    m_filters.emplace(std::string(id), FilterEntry { owned, std::string(connections) });
    return owned;
}

bool FilterCollection::RemoveFilter(std::string_view id) noexcept
{
    auto it = m_filters.find(id);
    if (it == m_filters.end())
        return false;

    m_filters.erase(it);

    return SaveToFile();
}

std::shared_ptr<Filter> FilterCollection::TryGetFilterById(std::string_view id) const noexcept
{
    auto it = m_filters.find(id);
    if (it == m_filters.end())
        return nullptr;

    return it->second.filter;
}

bool FilterCollection::SaveToFile() const noexcept
{
    const std::string tempFileName = std::string(m_fileName) + ".tmp";

    m_storage.RemoveFile(tempFileName); // ignore result

    for (auto& entry : m_filters)
    {
        if (!AppendFilterToFile(tempFileName, *entry.second.filter, entry.second.connections))
        {
            m_storage.RemoveFile(tempFileName);
            return false;
        }
    }

    if (!m_storage.RenameFile(tempFileName, m_fileName))
    {
        m_storage.RemoveFile(tempFileName);
        return false;
    }

    return true;
}

bool FilterCollection::AppendFilterToFile(std::string_view fileName, const Filter& filter, std::string_view connections) const noexcept
{
    IniWriter iniWriter;
    iniWriter.WriteSection("Filter");
    iniWriter.WriteKeyValue("Id", filter.GetId());
    iniWriter.WriteKeyValue("Type", GetFilterTypeName(filter.GetType()));
    iniWriter.WriteKeyValue("Connections", connections);
    
    std::string content = iniWriter.GetContent();
    content.reserve(content.size() + 1);
    content.push_back('\n');
    return m_storage.AppendFile(fileName, content);
}
