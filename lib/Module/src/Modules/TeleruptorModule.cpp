#include "TeleruptorModule.h"

#include <PinFactory.h>

TeleruptorModule::TeleruptorModule(const Bus& bus, const uint8_t address, const uint8_t numberOfTeleruptors) noexcept
    : Module(bus, address, ModuleType::Teleruptor)
    , m_numberOfTeleruptors(numberOfTeleruptors)
{
    m_teleruptorPins.reserve(m_numberOfTeleruptors);
    m_teleruptorFeedbackPins.reserve(m_numberOfTeleruptors);
    for (uint8_t i = 0; i < m_numberOfTeleruptors; ++i)
    {
        m_teleruptorPins.emplace_back(PinFactory::CreateInputPin<DigitalValue>(this));
        m_teleruptorFeedbackPins.emplace_back(PinFactory::CreateOutputPin<DigitalValue>());
    }

    m_inputPins.reserve(m_teleruptorPins.size());
    for (const auto& pin : m_teleruptorPins)
        m_inputPins.emplace_back(pin);

    m_outputPins.reserve(m_teleruptorFeedbackPins.size());
    for (const auto& pin : m_teleruptorFeedbackPins)
        m_outputPins.emplace_back(pin);
}

std::unique_ptr<TeleruptorModule> TeleruptorModule::CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept
{
    return std::make_unique<TeleruptorModule>(bus, address, initialData & 0x0F);
}

uint16_t TeleruptorModule::GenerateInitialData() const noexcept
{
    return static_cast<uint16_t>(m_numberOfTeleruptors);
}

ProcessResponse TeleruptorModule::Process() noexcept
{
    const bool forceDataExchange = !m_feedbackStateInSync;
    auto response = Exchange(0x06, forceDataExchange); // CMD6 - Request feedback state

    if (!response.Success) {
        return { .Success = false };
    }

    if (response.RespondedWithTypeAndData)
    {
        for (uint8_t i = 0; i < m_numberOfTeleruptors; ++i)
            m_teleruptorFeedbackPins[i]->SetState(DigitalValue((response.Data & (1 << i)) != 0));

        m_feedbackStateInSync = true;
    }

    return { .Success = true };
}

void TeleruptorModule::OnPinStateChanged(const Pin& pin) noexcept
{
    int8_t index = FindIndex(pin, m_teleruptorPins);
    if (index >= 0)
        UpdateTeleruptor(index, pin.GetStateAs<DigitalValue>());
}

void TeleruptorModule::UpdateTeleruptor(const uint8_t teleruptorIndex, const DigitalValue newValue) noexcept
{
    uint16_t command = newValue ? 0x01 : 0x02; // CMD1 - Set teleruptor ON, CMD2 - Set teleruptor OFF
    Exchange(command | (teleruptorIndex << 4), true);
}
