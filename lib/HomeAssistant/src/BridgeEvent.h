#pragma once

#include <Pin.h>

#include <cstddef>

struct BridgeEvent
{
    enum class Type {
        MqttConnected,
        MqttData,
        CompleteDeviceRegistration,
        UnregisterDevice,
        PublishState,
        Shutdown,
    } Type;

    char Id[32];            // For CompleteDeviceRegistration, UnregisterDevice & PublishState
    size_t IdLength;        // For CompleteDeviceRegistration, UnregisterDevice & PublishState
    PinState State;         // For PublishState

    char Topic[64];         // For MqttData
    size_t TopicLength;     // For MqttData
    char Payload[64];       // For MqttData
    size_t PayloadLength;   // For MqttData
};
