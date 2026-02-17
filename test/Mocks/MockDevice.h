#pragma once

#include <Devices/Device.h>
#include <string>
#include <cstring>

class MockDevice : public IDevice
{
public:
    MockDevice(std::string_view id = "test-device") noexcept
        : m_id(id)
        , m_subscribeToStateChangesCount(0)
        , m_processCommandCount(0)
        , m_lastCommand("")
        , m_lastSubtopic("")
    {
    }

    std::string_view GetId() const noexcept override
    {
        return m_id;
    }

    bool BuildDiscoveryTopic(char* buffer, size_t bufferLength) const noexcept override
    {
        const std::string topic = "homeassistant/switch/" + m_id + "/config";
        if (topic.length() >= bufferLength)
            return false;
        std::strcpy(buffer, topic.c_str());
        return true;
    }

    bool BuildDiscoveryPayload(char* buffer, size_t bufferLength) const noexcept override
    {
        const std::string payload = R"({"name":")" + m_id + R"(","command_topic":"domo/dev/)" + m_id + R"(/set"})";
        if (payload.length() >= bufferLength)
            return false;
        std::strcpy(buffer, payload.c_str());
        return true;
    }

    bool BuildStateMessages(StateMessageList& out) const noexcept override
    {
        StateMessage msg{};
        const std::string topic = "domo/dev/" + m_id + "/state";
        if (topic.length() >= sizeof(msg.Topic))
            return false;

        std::strcpy(msg.Topic, topic.c_str());
        std::strcpy(msg.Payload, "ON");
        msg.Retain = true;

        return out.Add(msg);
    }

    void SubscribeToStateChanges() noexcept override
    {
        m_subscribeToStateChangesCount++;
    }

    void ProcessCommand(std::string_view subtopic, std::string_view command) const noexcept override
    {
        const_cast<MockDevice*>(this)->m_lastSubtopic = std::string(subtopic);
        const_cast<MockDevice*>(this)->m_lastCommand = std::string(command);
        const_cast<MockDevice*>(this)->m_processCommandCount++;
    }

    // Helper methods for testing
    int GetSubscribeToStateChangesCount() const noexcept
    {
        return m_subscribeToStateChangesCount;
    }

    int GetProcessCommandCount() const noexcept
    {
        return m_processCommandCount;
    }

    const std::string& GetLastCommand() const noexcept
    {
        return m_lastCommand;
    }

    const std::string& GetLastSubtopic() const noexcept
    {
        return m_lastSubtopic;
    }

private:
    std::string m_id;
    int m_subscribeToStateChangesCount;
    int m_processCommandCount;
    std::string m_lastCommand;
    std::string m_lastSubtopic;
};
