#include "RelaisModule.h"

#include <PinFactory.h>

RelaisModule::RelaisModule(const Bus& bus, const uint8_t address, const uint8_t numberOfRelais) noexcept
    : Module(bus, address, ModuleType::Relais)
    , m_numberOfRelais(numberOfRelais)
{
    m_relaisPins.reserve(m_numberOfRelais);
    m_feedbackPins.reserve(m_numberOfRelais);
    for (uint8_t i = 0; i < m_numberOfRelais; ++i)
    {
        m_relaisPins.emplace_back(PinFactory::CreateInputPin<DigitalValue>(this));
        m_feedbackPins.emplace_back(PinFactory::CreateOutputPin<DigitalValue>());
    }

    m_inputPins.reserve(m_relaisPins.size());
    for (const auto& pin : m_relaisPins)
        m_inputPins.emplace_back(pin);

    m_outputPins.reserve(m_feedbackPins.size());
    for (const auto& pin : m_feedbackPins)
        m_outputPins.emplace_back(pin);
}

std::unique_ptr<RelaisModule> RelaisModule::CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept
{
    return std::make_unique<RelaisModule>(bus, address, initialData & 0x0F);
}

uint16_t RelaisModule::GenerateInitialData() const noexcept
{
    return static_cast<uint16_t>(m_numberOfRelais);
}

ProcessResponse RelaisModule::Process() noexcept
{
    const bool forceDataExchange = !m_feedbackStateInSync;
    auto response = Exchange(0x06, forceDataExchange); // CMD6 - Request feedback state

    if (!response.Success) {
        return { .Success = false };
    }

    if (response.RespondedWithTypeAndData)
    {
        for (uint8_t i = 0; i < m_numberOfRelais; ++i)
            m_feedbackPins[i]->SetState(DigitalValue((response.Data & (1 << i)) != 0));

        m_feedbackStateInSync = true;
    }

    return { .Success = true };
}

void RelaisModule::OnPinStateChanged(const Pin& pin) noexcept
{
    int8_t index = FindIndex(pin, m_relaisPins);
    if (index >= 0)
        UpdateRelais(index, pin.GetStateAs<DigitalValue>());
}

void RelaisModule::UpdateRelais(const uint8_t relaisIndex, const DigitalValue newValue) noexcept
{
    uint16_t command = newValue ? 0x01 : 0x02; // CMD1 - Set relais ON, CMD2 - Set relais OFF
    Exchange(command | (relaisIndex << 4), true);
}
