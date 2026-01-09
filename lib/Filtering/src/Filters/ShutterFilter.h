#pragma once

#include "Filter.h"

#include <memory>
#include <vector>

enum ShutterCommand
{
    Open,
    Close,
    Stop
};

class ShutterFilter final : public Filter
{
public:
    ShutterFilter();

private:
    ShutterCommand m_currentShutterCommand = ShutterCommand::Stop;
    std::shared_ptr<Pin> m_openInputPin;
    std::shared_ptr<Pin> m_closeInputPin;
    std::shared_ptr<Pin> m_openFeedbackInputPin;
    std::shared_ptr<Pin> m_closeFeedbackInputPin;
    std::shared_ptr<Pin> m_openOutputPin;
    std::shared_ptr<Pin> m_closeOutputPin;
};
