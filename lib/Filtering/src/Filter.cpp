#include "Filter.h"

Filter::Filter(const FilterType type, std::string_view id) noexcept
    : m_type(type)
{
    SetId(id);
}

FilterType Filter::GetType() const noexcept
{
    return m_type;
}

void Filter::WriteDescriptor(IniWriter& iniWriter) const noexcept
{
    iniWriter.WriteSection("Filter");
    iniWriter.WriteKeyValue("Type", GetFilterTypeName(GetType()));

    char key[16];
    char value[64];

    const auto inputPins = GetInputPins();
    for (std::size_t i = 0; i < inputPins.size(); ++i)
    {
        if (auto sharedPin = inputPins[i].lock())
        {
            const auto keyLen = snprintf(key, sizeof(key), "Input.%zu", i);
            const auto pinName = sharedPin->GetName();
            const auto valueLen = snprintf(value, sizeof(value), "%.*s,%s", (int)pinName.length(), pinName.data(), PinStateTypes[sharedPin->GetState().index()]);
            iniWriter.WriteKeyValue(std::string_view(key, keyLen), std::string_view(value, valueLen));
        }
    }

    const auto outputPins = GetOutputPins();
    for (std::size_t i = 0; i < outputPins.size(); ++i)
    {
        if (auto sharedPin = outputPins[i].lock())
        {
            const auto keyLen = snprintf(key, sizeof(key), "Output.%zu", i);
            const auto pinName = sharedPin->GetName();
            const auto valueLen = snprintf(value, sizeof(value), "%.*s,%s", (int)pinName.length(), pinName.data(), PinStateTypes[sharedPin->GetState().index()]);
            iniWriter.WriteKeyValue(std::string_view(key, keyLen), std::string_view(value, valueLen));
        }
    }
}

void Filter::WriteConfig(IniWriter& iniWriter) const noexcept
{
    iniWriter.WriteSection("Filter");
    iniWriter.WriteKeyValue("Id", GetId());
    iniWriter.WriteKeyValue("Type", GetFilterTypeName(GetType()));
}
