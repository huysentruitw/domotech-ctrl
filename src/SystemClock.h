#pragma once

#include <ctime>
#include <string>

class SystemClock final
{
public:
    SystemClock(const char* posixTimeZone) noexcept;

    void Init() const noexcept;

    struct tm GetCurrentTime() const noexcept;
    void WriteCurrentTime(std::string& output) const noexcept;

private:
    const char* m_posixTimeZone;
};
