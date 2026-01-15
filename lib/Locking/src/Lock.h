#pragma once

#ifndef NATIVE_BUILD

#include "freertos/FreeRTOS.h"

class Lock final
{
public:
    Lock(bool recursive = false) noexcept
        : m_recursive(recursive)
        , m_mutex(recursive ? xSemaphoreCreateRecursiveMutex() : xSemaphoreCreateMutex())
    {
    }

    void Take() const noexcept
    {
        m_recursive
            ? xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY)
            : xSemaphoreTake(m_mutex, portMAX_DELAY);
    }

    void Release() const noexcept
    {
        m_recursive
            ? xSemaphoreGiveRecursive(m_mutex)
            : xSemaphoreGive(m_mutex);
    }

private:
    const bool m_recursive;
    const SemaphoreHandle_t m_mutex;
};

#else

class Lock final
{
public:
    Lock(bool recursive = false) noexcept { }
    void Take() const noexcept { }
    void Release() const noexcept { }
};

#endif
