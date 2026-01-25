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

    char Id[32];            // For CompleteDeviceRegistration, UnregisterDevice
    size_t IdLength;        // For CompleteDeviceRegistration, UnregisterDevice

    char Topic[64];         // For MqttData & PublishState
    size_t TopicLength;     // For MqttData & PublishState
    char Payload[64];       // For MqttData & PublishState
    size_t PayloadLength;   // For MqttData & PublishState

    bool Retain;            // For PublishState
};
