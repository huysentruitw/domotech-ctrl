#pragma once

#include "../Module.h"

#include <memory>
#include <vector>

class InputModule final : public Module
{
public:
    InputModule(const Bus& bus, const uint8_t address, const uint8_t numberOfInputs) noexcept;

    static std::unique_ptr<InputModule> CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept;
    uint16_t GenerateInitialData() const noexcept override;

    ProcessResponse Process() noexcept override;

private:
    const uint8_t m_numberOfInputs;
    bool m_stateInSync = false;

    std::vector<std::shared_ptr<Pin>> m_pins;

    DigitalValue MapInputState(const uint8_t buttonIndex, const uint16_t data) const noexcept;
};
