#pragma once

class ScanLed final
{
public:
    ScanLed() noexcept;

    void Toggle() const noexcept;

private:
    mutable bool m_state;    
};
