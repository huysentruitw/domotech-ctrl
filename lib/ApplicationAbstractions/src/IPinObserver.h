#pragma once

class Pin; // Forward declaration

class IPinObserver
{
public:
    virtual ~IPinObserver() = default;

    virtual void OnPinStateChanged(const Pin& pin) noexcept = 0;
};
