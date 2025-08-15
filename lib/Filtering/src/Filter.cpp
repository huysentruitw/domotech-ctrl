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
            const std::string stateTypeName = PinStateTypes[sharedPin->GetState().index()];
            iniWriter.WriteKeyValue("Input." + std::to_string(i), sharedPin->GetName() + "," + stateTypeName);
        }
    }

    const auto outputPins = GetOutputPins();
    for (std::size_t i = 0; i < outputPins.size(); ++i) {
        if (auto sharedPin = outputPins[i].lock()) {
            const std::string stateTypeName = PinStateTypes[sharedPin->GetState().index()];
            iniWriter.WriteKeyValue("Output." + std::to_string(i), sharedPin->GetName() + "," + stateTypeName);
        }
    }
}

void Filter::WriteConfig(IniWriter& iniWriter) const
{
    iniWriter.WriteSection("Filter");
    iniWriter.WriteKeyValue("Type", GetFilterTypeName(GetType()));
    iniWriter.WriteKeyValue("Name", GetName());
}
