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
    ShutterFilter();

    std::vector<std::weak_ptr<Pin>> GetInputPins() const override;
    std::vector<std::weak_ptr<Pin>> GetOutputPins() const override;

private:
    ShutterCommand m_currentShutterCommand = ShutterCommand::Stop;
    std::shared_ptr<Pin> m_inputOpenPin;
    std::shared_ptr<Pin> m_inputClosePin;
    std::shared_ptr<Pin> m_inputOpenFeedbackPin;
    std::shared_ptr<Pin> m_inputCloseFeedbackPin;
    std::shared_ptr<Pin> m_outputOpenPin;
    std::shared_ptr<Pin> m_outputClosePin;
};
