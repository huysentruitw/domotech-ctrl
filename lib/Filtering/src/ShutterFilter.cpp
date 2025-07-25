#include "ShutterFilter.h"

#include <PinFactory.h>

ShutterFilter::ShutterFilter()
{
    m_inputOpenPin = PinFactory::CreateInputPin<DigitalValue>(
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
            {
                m_currentShutterCommand = ShutterCommand::Open;

                if (m_inputCloseFeedbackPin->GetStateAs<DigitalValue>() == DigitalValue(true))
                {
                    m_outputClosePin->SetState(DigitalValue(false));
                }
                else
                {
                    m_outputOpenPin->SetState(DigitalValue(true));
                }
            }
            else
            {
                m_currentShutterCommand = ShutterCommand::Stop;
                m_outputOpenPin->SetState(DigitalValue(false));
                m_outputClosePin->SetState(DigitalValue(false));
            }
        });

    m_inputClosePin = PinFactory::CreateInputPin<DigitalValue>(
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
            {
                m_currentShutterCommand = ShutterCommand::Close;

                if (m_inputOpenFeedbackPin->GetStateAs<DigitalValue>() == DigitalValue(true))
                {
                    m_outputOpenPin->SetState(DigitalValue(false));
                }
                else
                {
                    m_outputClosePin->SetState(DigitalValue(true));
                }
            }
            else
            {
                m_currentShutterCommand = ShutterCommand::Stop;
                m_outputOpenPin->SetState(DigitalValue(false));
                m_outputClosePin->SetState(DigitalValue(false));
            }
        });

    m_inputOpenFeedbackPin = PinFactory::CreateInputPin<DigitalValue>(
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(false) && m_currentShutterCommand == ShutterCommand::Close)
            {
                m_outputClosePin->SetState(DigitalValue(true));
            }
        });

    m_inputCloseFeedbackPin = PinFactory::CreateInputPin<DigitalValue>(
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(false) && m_currentShutterCommand == ShutterCommand::Open)
            {
                m_outputOpenPin->SetState(DigitalValue(true));
            }
        });

    m_outputOpenPin = PinFactory::CreateOutputPin<DigitalValue>();
    m_outputClosePin = PinFactory::CreateOutputPin<DigitalValue>();
}

std::vector<std::weak_ptr<Pin>> ShutterFilter::GetInputPins() const
{
    return {m_inputOpenPin, m_inputClosePin, m_inputOpenFeedbackPin, m_inputCloseFeedbackPin};
}

std::vector<std::weak_ptr<Pin>> ShutterFilter::GetOutputPins() const
{
    return {m_outputOpenPin, m_outputClosePin};
}
