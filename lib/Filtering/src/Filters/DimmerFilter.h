#pragma once

#include <IPinObserver.h>

#include "Filter.h"

#include <memory>
#include <vector>

class DimmerFilter final : public Filter, private IPinObserver
{
public:
    DimmerFilter(std::string_view id = {}) noexcept;

    uint8_t GetLastOnPercentage() noexcept;
    void SetState(DimmerControlValue state) noexcept;

    std::weak_ptr<Pin> GetToggleInputPin() const noexcept { return m_toggleInputPin; }
    std::weak_ptr<Pin> GetControlOutputPin() const noexcept { return m_controlOutputPin; }
    std::weak_ptr<Pin> GetFeedbackOutputPin() const noexcept { return m_feedbackOutputPin; }

private:
    uint8_t m_lastOnPercentage = 100;
    bool m_isPressed = false;
    uint8_t m_pressDurationInHundredMs = 0;  // Tracks press duration in 100ms units
    bool m_isLongPress = false;                // True when press duration exceeds 300ms
    bool m_isDimming = false;                  // True when currently dimming
    int8_t m_dimmingDirection = 1;             // 1 for up, -1 for down

    std::shared_ptr<Pin> m_toggleInputPin;
    std::shared_ptr<Pin> m_controlOutputPin;
    std::shared_ptr<Pin> m_feedbackOutputPin;

    void Every100Ms() noexcept override;
    void OnPinStateChanged(const Pin& pin) noexcept override;
    void HandleDimming() noexcept;
};
