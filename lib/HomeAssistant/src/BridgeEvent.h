#pragma once

#include <Pin.h>

#include <cstddef>

struct BridgeEvent
{
    enum class Type {
        MqttConnected,
        MqttData,
        CompleteFilterRegistration,
        UnregisterFilter,
        PublishState,
        Shutdown,
    } Type;

    char Id[32];            // For CompleteFilterRegistration, UnregisterFilter & PublishState
    size_t IdLength;        // For CompleteFilterRegistration, UnregisterFilter & PublishState
    PinState State;         // For PublishState

    char Topic[64];         // For MqttData
    size_t TopicLength;     // For MqttData
    char Payload[64];       // For MqttData
    size_t PayloadLength;   // For MqttData
};
