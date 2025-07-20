#pragma once

#include "FilterBase.h"

#include <memory>
#include <vector>

class ToggleFilter : public FilterBase
{
public:
    ToggleFilter()
    {
        m_inputPin = std::make_shared<InputPin<DigitalValue>>(
            [this](const DigitalValue& value) {
                if (value) {
                    m_outputPin->SetState(DigitalValue(!m_outputPin->GetState()));
                }
            },
            DigitalValue(false)
        );

        m_outputPin = std::make_shared<OutputPin<DigitalValue>>(DigitalValue(false));
    }

    std::vector<std::weak_ptr<InputPin<DigitalValue>>> GetDigitalInputPins() const override { return { m_inputPin }; }
    std::vector<std::weak_ptr<OutputPin<DigitalValue>>> GetDigitalOutputPins() const override { return { m_outputPin }; }

private:
    std::shared_ptr<InputPin<DigitalValue>> m_inputPin;
    std::shared_ptr<OutputPin<DigitalValue>> m_outputPin;
};
