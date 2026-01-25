#pragma once

#include <IPinObserver.h>

#include "Filter.h"

#include <memory>
#include <vector>

enum ShutterCommand
{
    Open,
    Close,
    Stop
};

class ShutterFilter final : public Filter, private IPinObserver
{
public:
    ShutterFilter(std::string_view id = {}) noexcept;

    void Open() noexcept;
    void Close() noexcept;
    void Stop() noexcept;

    // For testing
    void MoveSignalStartMs(int offsetMs) noexcept;

private:
    ShutterCommand m_currentShutterCommand = ShutterCommand::Stop;
    std::shared_ptr<Pin> m_openInputPin;
    std::shared_ptr<Pin> m_closeInputPin;
    std::shared_ptr<Pin> m_stopInputPin;
    std::shared_ptr<Pin> m_openFeedbackInputPin;
    std::shared_ptr<Pin> m_closeFeedbackInputPin;
    std::shared_ptr<Pin> m_openOutputPin;
    std::shared_ptr<Pin> m_closeOutputPin;

    uint64_t m_signalStartMs = 0;
    static uint64_t GetMsSinceBoot() noexcept;

    void OnPinStateChanged(const Pin& pin) noexcept override;
};
