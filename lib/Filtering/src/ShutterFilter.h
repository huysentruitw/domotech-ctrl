#pragma once

#include "FilterBase.h"

#include <memory>
#include <vector>

enum ShutterCommand
{
    Open,
    Close,
    Stop
};

class ShutterFilter : public FilterBase
{
public:
    ShutterFilter()
    {
        m_inputOpenPin = std::make_shared<InputPin<DigitalValue>>(
            [this](const DigitalValue& value) {
                if (value) {
                    m_currentShutterCommand = ShutterCommand::Open;

                    if (m_inputCloseFeedbackPin->GetState()) {
                        m_outputClosePin->SetState(DigitalValue(false));
                    } else {
                        m_outputOpenPin->SetState(DigitalValue(true));
                    }
                } else {
                    m_currentShutterCommand = ShutterCommand::Stop;
                    m_outputOpenPin->SetState(DigitalValue(false));
                    m_outputClosePin->SetState(DigitalValue(false));
                }
            },
            DigitalValue(false)
        );

        m_inputClosePin = std::make_shared<InputPin<DigitalValue>>(
            [this](const DigitalValue& value) {
                if (value) {
                    m_currentShutterCommand = ShutterCommand::Close;

                    if (m_inputOpenFeedbackPin->GetState()) {
                        m_outputOpenPin->SetState(DigitalValue(false));
                    } else {
                        m_outputClosePin->SetState(DigitalValue(true));
                    }
                } else {
                    m_currentShutterCommand = ShutterCommand::Stop;
                    m_outputOpenPin->SetState(DigitalValue(false));
                    m_outputClosePin->SetState(DigitalValue(false));
                }
            },
            DigitalValue(false)
        );

        m_inputOpenFeedbackPin = std::make_shared<InputPin<DigitalValue>>(
            [this](const DigitalValue& value) {
                if (!value && m_currentShutterCommand == ShutterCommand::Close) {
                    m_outputClosePin->SetState(DigitalValue(true));
                }
            },
            DigitalValue(false)
        );

        m_inputCloseFeedbackPin = std::make_shared<InputPin<DigitalValue>>(
            [this](const DigitalValue& value) {
                if (!value && m_currentShutterCommand == ShutterCommand::Open) {
                    m_outputOpenPin->SetState(DigitalValue(true));
                }
            },
            DigitalValue(false)
        );

        m_outputOpenPin = std::make_shared<OutputPin<DigitalValue>>(DigitalValue(false));
        m_outputClosePin = std::make_shared<OutputPin<DigitalValue>>(DigitalValue(false));
    }

    std::vector<std::weak_ptr<InputPin<DigitalValue>>> GetDigitalInputPins() const override { return { m_inputOpenPin, m_inputClosePin, m_inputOpenFeedbackPin, m_inputCloseFeedbackPin }; }
    std::vector<std::weak_ptr<OutputPin<DigitalValue>>> GetDigitalOutputPins() const override { return { m_outputOpenPin, m_outputClosePin }; }

private:
    ShutterCommand m_currentShutterCommand = ShutterCommand::Stop;
    std::shared_ptr<InputPin<DigitalValue>> m_inputOpenPin;
    std::shared_ptr<InputPin<DigitalValue>> m_inputClosePin;
    std::shared_ptr<InputPin<DigitalValue>> m_inputOpenFeedbackPin;
    std::shared_ptr<InputPin<DigitalValue>> m_inputCloseFeedbackPin;
    std::shared_ptr<OutputPin<DigitalValue>> m_outputOpenPin;
    std::shared_ptr<OutputPin<DigitalValue>> m_outputClosePin;
};
