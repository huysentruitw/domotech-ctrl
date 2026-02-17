#pragma once

class IMqttClient
{
public:
    virtual ~IMqttClient() noexcept = default;

    virtual void Subscribe(const char* topic) const noexcept = 0;
    virtual void Publish(const char* topic, const char* payload, bool retain) const noexcept = 0;
};
