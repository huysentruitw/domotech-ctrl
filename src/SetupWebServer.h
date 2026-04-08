#pragma once

class SetupWebServer final
{
public:
    SetupWebServer() = delete; // Prevent instantiation of static class

    static void Start() noexcept;
};
