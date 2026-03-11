#include "SystemClock.h"

#include "esp_sntp.h"

SystemClock::SystemClock(const char* posixTimeZone) noexcept
    : m_posixTimeZone(posixTimeZone)
{
}

void SystemClock::Init() const noexcept
{
    setenv("TZ", m_posixTimeZone, 1);
    tzset();
    
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

struct tm SystemClock::GetCurrentTime() const noexcept
{
    time_t now;
    time(&now);

    struct tm timeInfo;
    localtime_r(&now, &timeInfo);

    return timeInfo;
}

void SystemClock::WriteCurrentTime(std::string& output) const noexcept
{
    struct tm currentTime = GetCurrentTime();
    char buffer[32];
    size_t len = strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &currentTime);
    output.append(buffer, len);
}
