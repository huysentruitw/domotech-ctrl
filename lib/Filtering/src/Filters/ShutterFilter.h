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

    std::weak_ptr<Pin> GetOpenInputPin() const noexcept { return m_openInputPin; };
    std::weak_ptr<Pin> GetCloseInputPin() const noexcept { return m_closeInputPin; };
    std::weak_ptr<Pin> GetStopInputPin() const noexcept { return m_stopInputPin; };
    std::weak_ptr<Pin> GetOpenFeedbackInputPin() const noexcept { return m_openFeedbackInputPin; };
    std::weak_ptr<Pin> GetCloseFeedbackInputPin() const noexcept { return m_closeFeedbackInputPin; };
    std::weak_ptr<Pin> GetOpenOutputPin() const noexcept { return m_openOutputPin; };
    std::weak_ptr<Pin> GetCloseOutputPin() const noexcept { return m_closeOutputPin; };

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
