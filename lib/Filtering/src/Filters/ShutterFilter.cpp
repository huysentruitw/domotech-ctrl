#include "ShutterFilter.h"

#include <PinFactory.h>

ShutterFilter::ShutterFilter(std::string_view id) noexcept
    : Filter(FilterType::Shutter, id)
{
    m_openInputPin = PinFactory::CreateInputPin<DigitalValue>("Open", this);
    m_closeInputPin = PinFactory::CreateInputPin<DigitalValue>("Close", this);
    m_stopInputPin = PinFactory::CreateInputPin<DigitalValue>("Stop", this);
    m_openFeedbackInputPin = PinFactory::CreateInputPin<DigitalValue>("OpenFeedback", this);
    m_closeFeedbackInputPin = PinFactory::CreateInputPin<DigitalValue>("CloseFeedback", this);

    m_openOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Open");
    m_closeOutputPin = PinFactory::CreateOutputPin<DigitalValue>("Close");

    m_inputPins = { m_openInputPin, m_closeInputPin, m_stopInputPin, m_openFeedbackInputPin, m_closeFeedbackInputPin };
    m_outputPins = { m_openOutputPin, m_closeOutputPin };
}

void ShutterFilter::Every100Ms() noexcept
{
    if (m_signalDurationInMs >= 0 && m_signalDurationInMs < 10000)
        m_signalDurationInMs += 100;
}

void ShutterFilter::Open() noexcept
{
    m_currentShutterCommand = ShutterCommand::Open;

    if (m_closeFeedbackInputPin->GetStateAs<DigitalValue>() == DigitalValue(true))
        m_closeOutputPin->SetState(DigitalValue(false));
    else
        m_openOutputPin->SetState(DigitalValue(true));
}

void ShutterFilter::Close() noexcept
{
    m_currentShutterCommand = ShutterCommand::Close;

    if (m_openFeedbackInputPin->GetStateAs<DigitalValue>() == DigitalValue(true))
        m_openOutputPin->SetState(DigitalValue(false));
    else
        m_closeOutputPin->SetState(DigitalValue(true));
}

void ShutterFilter::Stop() noexcept
{
    m_currentShutterCommand = ShutterCommand::Stop;
    m_openOutputPin->SetState(DigitalValue(false));
    m_closeOutputPin->SetState(DigitalValue(false));
}

void ShutterFilter::OnPinStateChanged(const Pin& pin) noexcept
{
    if (pin == m_openInputPin)
    {
        if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
        {
            Open();
            m_signalDurationInMs = 0;
        }
        else
        {
            if (m_currentShutterCommand == ShutterCommand::Open && m_signalDurationInMs >= 1000)
            {
                // Stop the shutter when released after long press
                Stop();
            }

            m_signalDurationInMs = -1;
        }
    }
    else if (pin == m_closeInputPin)
    {
        if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
        {
            Close();
            m_signalDurationInMs = 0;
        }
        else
        {
            if (m_currentShutterCommand == ShutterCommand::Close && m_signalDurationInMs >= 1000)
            {
                // Stop the shutter when released after long press
                Stop();
            }

            m_signalDurationInMs = -1;
        }
    }
    else if (pin == m_stopInputPin)
    {
        if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
        {
            Stop();

            m_signalDurationInMs = -1;
        }
    }
    else if (pin == m_openFeedbackInputPin)
    {
        if (pin.GetStateAs<DigitalValue>() == DigitalValue(false) && m_currentShutterCommand == ShutterCommand::Close)
            m_closeOutputPin->SetState(DigitalValue(true));
    }
    else if (pin == m_closeFeedbackInputPin)
    {
        if (pin.GetStateAs<DigitalValue>() == DigitalValue(false) && m_currentShutterCommand == ShutterCommand::Open)
            m_openOutputPin->SetState(DigitalValue(true));
    }
}
