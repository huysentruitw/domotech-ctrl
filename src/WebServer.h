#pragma once

#include "SystemClock.h"

#include <IStorage.h>
#include <Manager.h>
#include <Wifi.h>

#include <string_view>

class WebServer final
{
public:
    WebServer() = delete; // Prevent instantiation of static class

    static void Start(std::string_view version, IStorage& storage, Manager& manager, SystemClock& systemClock, Wifi& wifi) noexcept;

private:
    std::string_view m_version;
    IStorage& m_storage;
};
