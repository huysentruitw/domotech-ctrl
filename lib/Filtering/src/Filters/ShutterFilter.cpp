#include "ShutterFilter.h"

#include <PinFactory.h>

ShutterFilter::ShutterFilter()
    : Filter(FilterType::Shutter)
{
    m_openInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Open",
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
            {
                m_currentShutterCommand = ShutterCommand::Open;

                if (m_closeFeedbackInputPin->GetStateAs<DigitalValue>() == DigitalValue(true))
                {
                    m_closeOutputPin->SetState(DigitalValue(false));
                }
                else
                {
                    m_openOutputPin->SetState(DigitalValue(true));
                }
            }
            else
            {
                m_currentShutterCommand = ShutterCommand::Stop;
                m_openOutputPin->SetState(DigitalValue(false));
                m_closeOutputPin->SetState(DigitalValue(false));
            }
        });

    m_closeInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Close",
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
            {
                m_currentShutterCommand = ShutterCommand::Close;

                if (m_openFeedbackInputPin->GetStateAs<DigitalValue>() == DigitalValue(true))
                {
                    m_openOutputPin->SetState(DigitalValue(false));
                }
                else
                {
                    m_closeOutputPin->SetState(DigitalValue(true));
                }
            }
            else
            {
                m_currentShutterCommand = ShutterCommand::Stop;
                m_openOutputPin->SetState(DigitalValue(false));
                m_closeOutputPin->SetState(DigitalValue(false));
            }
        });

    m_openFeedbackInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "OpenFeedback",
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(false) && m_currentShutterCommand == ShutterCommand::Close)
            {
                m_closeOutputPin->SetState(DigitalValue(true));
            }
        });

    m_closeFeedbackInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "CloseFeedback",
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(false) && m_currentShutterCommand == ShutterCommand::Open)
            {
                m_openOutputPin->SetState(DigitalValue(true));
            }
        });

    m_openOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Open");
    m_closeOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Close");

    m_inputPins = { m_openInputPin, m_closeInputPin, m_openFeedbackInputPin, m_closeFeedbackInputPin };
    m_outputPins = { m_openOutputPin, m_closeOutputPin };
}
