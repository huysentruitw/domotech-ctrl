#include "ShutterFilter.h"

#include <PinFactory.h>

#ifndef NATIVE_BUILD
    #include "esp_timer.h"
#else
    #define esp_timer_get_time() 0
#endif

ShutterFilter::ShutterFilter(std::string_view id)
    : Filter(FilterType::Shutter, id)
{
    m_openInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Open",
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
            {
                m_currentShutterCommand = ShutterCommand::Open;
                m_signalStartMs = GetMsSinceBoot();

                if (m_closeFeedbackInputPin->GetStateAs<DigitalValue>() == DigitalValue(true))
                {
                    m_closeOutputPin->SetState(DigitalValue(false));
                }
                else
                {
                    m_openOutputPin->SetState(DigitalValue(true));
                }
            }
            else if (m_currentShutterCommand == ShutterCommand::Open && (GetMsSinceBoot() - m_signalStartMs) <= 1000)
            {
                // Stop the shutter when released within 1 second
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
                m_signalStartMs = GetMsSinceBoot();

                if (m_openFeedbackInputPin->GetStateAs<DigitalValue>() == DigitalValue(true))
                {
                    m_openOutputPin->SetState(DigitalValue(false));
                }
                else
                {
                    m_closeOutputPin->SetState(DigitalValue(true));
                }
            }
            else if (m_currentShutterCommand == ShutterCommand::Close && (GetMsSinceBoot() - m_signalStartMs) <= 1000)
            {
                // Stop the shutter when released within 1 second
                m_currentShutterCommand = ShutterCommand::Stop;
                m_openOutputPin->SetState(DigitalValue(false));
                m_closeOutputPin->SetState(DigitalValue(false));
            }
        });

    m_stopInputPin = PinFactory::CreateInputPin<DigitalValue>(
        "Stop",
        [this](const Pin &pin)
        {
            if (pin.GetStateAs<DigitalValue>() == DigitalValue(true))
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

    m_inputPins = { m_openInputPin, m_closeInputPin, m_stopInputPin, m_openFeedbackInputPin, m_closeFeedbackInputPin };
    m_outputPins = { m_openOutputPin, m_closeOutputPin };
}

uint64_t ShutterFilter::GetMsSinceBoot() noexcept
{
    return static_cast<uint64_t>(esp_timer_get_time()) / 1000ULL;
}
