#pragma once

#include "Lock.h"

class LockGuard final
{
public:
    explicit LockGuard(const Lock& lock) noexcept : m_lock(lock) { m_lock.Take(); }
    ~LockGuard() noexcept { m_lock.Release(); }

private:
    const Lock& m_lock;
};
