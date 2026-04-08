#pragma once

#include <IPinObserver.h>

#include "../Module.h"

#include <memory>
#include <vector>

class RelaisModule final : public Module, private IPinObserver
{
public:
    RelaisModule(const Bus& bus, const uint8_t address, const uint8_t numberOfRelais) noexcept;

    static std::unique_ptr<RelaisModule> CreateFromInitialData(const Bus& bus, const uint8_t address, const uint16_t initialData) noexcept;
    uint16_t GenerateInitialData() const noexcept override;

    ProcessResponse Process() noexcept override;

private:
    const uint8_t m_numberOfRelais;
    bool m_feedbackStateInSync = false;
    std::vector<std::shared_ptr<Pin>> m_relaisPins;
    std::vector<std::shared_ptr<Pin>> m_feedbackPins;

    void OnPinStateChanged(const Pin& pin) noexcept override;
    void UpdateRelais(const uint8_t relaisIndex, const DigitalValue newValue) noexcept;
};
