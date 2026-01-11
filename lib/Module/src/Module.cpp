#include "Module.h"

Module::Module(const Bus& bus, const uint8_t address, const ModuleType type)
    : m_bus(bus)
    , m_address(address)
    , m_type(type)
{
    char id[8];
    int len = snprintf(id, sizeof(id), "A%u", m_address);
    SetId(std::string_view(id, len));
}

ScanResponse Module::Poll() const
{
    return m_bus.Poll(m_address);
}

ScanResponse Module::Exchange(const uint16_t data) const
{
    return m_bus.Exchange(m_address, data);
}

uint8_t Module::GetAddress() const
{
    return m_address;
}

ModuleType Module::GetType() const
{
    return m_type;
}

void Module::WriteConfig(IniWriter& iniWriter) const
{
    iniWriter.WriteSection("Module");
    iniWriter.WriteKeyValue("Id", GetId());
    iniWriter.WriteKeyValue("Type", GetModuleTypeName(GetType()));
    iniWriter.WriteKeyValue("Address", std::to_string(GetAddress()));

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
