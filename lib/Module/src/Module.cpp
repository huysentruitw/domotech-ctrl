#include "Module.h"

Module::Module(const Bus& bus, const uint8_t address, const ModuleType type) noexcept
    : m_bus(bus)
    , m_address(address)
    , m_type(type)
{
    char id[8];
    int len = snprintf(id, sizeof(id), "A%u", m_address);
    SetId(std::string_view(id, len));
}

ScanResponse Module::Poll() const noexcept
{
    return m_bus.Poll(m_address);
}

ScanResponse Module::Exchange(const uint16_t data) const noexcept
{
    return m_bus.Exchange(m_address, data);
}

uint8_t Module::GetAddress() const noexcept
{
    return m_address;
}

ModuleType Module::GetType() const noexcept
{
    return m_type;
}

void Module::WriteConfig(IniWriter& iniWriter) const noexcept
{
    iniWriter.WriteSection("Module");
    iniWriter.WriteKeyValue("Id", GetId());
    iniWriter.WriteKeyValue("Type", GetModuleTypeName(GetType()));
    iniWriter.WriteKeyValue("Address", std::to_string(GetAddress()));

    char key[16];
    char value[64];

    const auto inputPins = GetInputPins();
    for (std::size_t i = 0; i < inputPins.size(); ++i)
    {
        if (auto sharedPin = inputPins[i].lock())
        {
            const auto keyLen = snprintf(key, sizeof(key), "Input.%zu", i);
            const auto valueLen = snprintf(value, sizeof(value), "%s,%s", sharedPin->GetName().c_str(), PinStateTypes[sharedPin->GetState().index()]);
            iniWriter.WriteKeyValue(std::string_view(key, keyLen), std::string_view(value, valueLen));
        }
    }

    const auto outputPins = GetOutputPins();
    for (std::size_t i = 0; i < outputPins.size(); ++i)
    {
        if (auto sharedPin = outputPins[i].lock())
        {
            const auto keyLen = snprintf(key, sizeof(key), "Output.%zu", i);
            const auto valueLen = snprintf(value, sizeof(value), "%s,%s", sharedPin->GetName().c_str(), PinStateTypes[sharedPin->GetState().index()]);
            iniWriter.WriteKeyValue(std::string_view(key, keyLen), std::string_view(value, valueLen));
        }
    }
}
