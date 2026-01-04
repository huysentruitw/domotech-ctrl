#include "Filter.h"

Filter::Filter(const FilterType type)
    : m_type(type)
{
}

FilterType Filter::GetType() const
{
    return m_type;
}

void Filter::WriteDescriptor(IniWriter& iniWriter) const
{
    iniWriter.WriteSection("Filter");
    iniWriter.WriteKeyValue("Type", GetFilterTypeName(GetType()));

    const auto inputPins = GetInputPins();
    for (std::size_t i = 0; i < inputPins.size(); ++i) {
        if (auto sharedPin = inputPins[i].lock()) {
            std::string value = sharedPin->GetName();
            value.append(",");
            value.append(PinStateTypes[sharedPin->GetState().index()]);
            iniWriter.WriteKeyValue("Input." + std::to_string(i), value);
        }
    }

    const auto outputPins = GetOutputPins();
    for (std::size_t i = 0; i < outputPins.size(); ++i) {
        if (auto sharedPin = outputPins[i].lock()) {
            std::string value = sharedPin->GetName();
            value.append(",");
            value.append(PinStateTypes[sharedPin->GetState().index()]);
            iniWriter.WriteKeyValue("Output." + std::to_string(i), value);
        }
    }
}

void Filter::WriteConfig(IniWriter& iniWriter) const
{
    iniWriter.WriteSection("Filter");
    iniWriter.WriteKeyValue("Type", GetFilterTypeName(GetType()));
    iniWriter.WriteKeyValue("Name", GetName());
}
