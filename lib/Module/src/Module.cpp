#include "Module.h"

Module::Module(const Bus& bus, const uint8_t address, const ModuleType type) noexcept
    : m_bus(bus)
    , m_address(address)
    , m_type(type)
    , m_defaultPriority(8)
{
    char id[8];
    int len = snprintf(id, sizeof(id), "A%u", m_address);
    SetId(std::string_view(id, len));
    m_priority = m_defaultPriority;
}

void Module::Tick(uint16_t tick) noexcept
{
    if ((tick + m_address) % m_priority != 0)
        return;

    auto response = Process();

    if (response.Success)
    {
        m_priority = response.BoostPriority ? 1 : m_defaultPriority;
    }
    else
    {
        m_priority = (uint8_t)std::min<uint16_t>(m_priority << 1, 128);
    }
}

ScanResponse Module::Exchange(const uint16_t data, const bool forceDataExchange) const noexcept
{
    return m_bus.Exchange(m_address, data, forceDataExchange);
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
