#pragma once

#include <IMqttClient.h>
#include <vector>
#include <string>
#include <cstring>

struct MqttMessage
{
    std::string topic;
    std::string payload;
    bool retain;

    MqttMessage(const char* t, const char* p, bool r)
        : topic(t), payload(p), retain(r)
    {
    }
};

class MockMqttClient : public IMqttClient
{
public:
    MockMqttClient() noexcept
        : m_subscriptions(), m_publications()
    {
    }

    void Subscribe(const char* topic) const noexcept override
    {
        const_cast<MockMqttClient*>(this)->m_subscriptions.emplace_back(topic);
    }

    void Publish(const char* topic, const char* payload, bool retain) const noexcept override
    {
        const_cast<MockMqttClient*>(this)->m_publications.emplace_back(topic, payload, retain);
    }

    // Helper methods for testing
    size_t GetSubscriptionCount() const noexcept
    {
        return m_subscriptions.size();
    }

    const std::string& GetSubscriptionAt(size_t index) const noexcept
    {
        return m_subscriptions[index];
    }

    bool HasSubscription(const std::string& topic) const noexcept
    {
        for (const auto& sub : m_subscriptions)
        {
            if (sub == topic)
                return true;
        }
        return false;
    }

    size_t GetPublicationCount() const noexcept
    {
        return m_publications.size();
    }

    const MqttMessage& GetPublicationAt(size_t index) const noexcept
    {
        return m_publications[index];
    }

    bool HasPublicationWithTopic(const std::string& topic) const noexcept
    {
        for (const auto& pub : m_publications)
        {
            if (pub.topic == topic)
                return true;
        }
        return false;
    }

    const MqttMessage* FindPublicationByTopic(const std::string& topic) const noexcept
    {
        for (const auto& pub : m_publications)
        {
            if (pub.topic == topic)
                return &pub;
        }
        return nullptr;
    }

    void ClearAll() noexcept
    {
        m_subscriptions.clear();
        m_publications.clear();
    }

private:
    std::vector<std::string> m_subscriptions;
    std::vector<MqttMessage> m_publications;
};
