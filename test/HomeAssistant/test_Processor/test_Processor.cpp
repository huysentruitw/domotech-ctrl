#include <unity.h>
#include <memory>

#include <Processor.h>
#include "../../Mocks/MockEventBus.h"
#include "../../Mocks/MockMqttClient.h"
#include "../../Mocks/MockDevice.h"

static MockMqttClient mockMqttClient;
static MockEventBus mockEventBus;

void setUp(void)
{
    mockMqttClient.ClearAll();
    mockEventBus.ClearEvents();
}

void tearDown(void)
{
    // Cleanup after each test
}

// Test: RegisterDevice enqueues CompleteDeviceRegistration event
void Processor_RegisterDevice_EnqueuesCompleteDeviceRegistrationEvent()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device = std::make_shared<MockDevice>("test-device-1");

    // Act
    processor.RegisterDevice(device);

    // Assert
    TEST_ASSERT_EQUAL(1, mockEventBus.GetEventCount());
    const auto& event = mockEventBus.GetLastEvent();
    TEST_ASSERT_EQUAL(BridgeEvent::Type::CompleteDeviceRegistration, event.Type);
    TEST_ASSERT_EQUAL(13, event.IdLength); // "test-device-1"
}

// Test: UnregisterDevice enqueues UnregisterDevice event
void Processor_UnregisterDevice_EnqueuesUnregisterDeviceEvent()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);

    // Act
    processor.UnregisterDevice("test-device-1");

    // Assert
    TEST_ASSERT_EQUAL(1, mockEventBus.GetEventCount());
    const auto& event = mockEventBus.GetLastEvent();
    TEST_ASSERT_EQUAL(BridgeEvent::Type::UnregisterDevice, event.Type);
    TEST_ASSERT_EQUAL(13, event.IdLength);
}

// Test: Process MqttConnected subscribes to topic
void Processor_ProcessMqttConnected_SubscribesToTopic()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    BridgeEvent event{};
    event.Type = BridgeEvent::Type::MqttConnected;

    // Act
    processor.Process(event);

    // Assert
    TEST_ASSERT_EQUAL(1, mockMqttClient.GetSubscriptionCount());
    TEST_ASSERT_TRUE(mockMqttClient.HasSubscription("domo/dev/#"));
}

// Test: Process MqttConnected with registered devices enqueues discovery
void Processor_ProcessMqttConnected_WithRegisteredDevices_EnqueuesDiscovery()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device = std::make_shared<MockDevice>("device-1");
    processor.RegisterDevice(device);
    mockEventBus.ClearEvents();

    BridgeEvent connectedEvent{};
    connectedEvent.Type = BridgeEvent::Type::MqttConnected;

    // Act
    processor.Process(connectedEvent);

    // Assert
    TEST_ASSERT_TRUE(mockEventBus.HasEventWithTypeValue(
        static_cast<int>(BridgeEvent::Type::PublishNextDiscovery)));
}

// Test: Process CompleteDeviceRegistration publishes discovery and subscribes
void Processor_ProcessCompleteDeviceRegistration_PublishesDiscoveryAndState()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device = std::make_shared<MockDevice>("device-1");
    processor.RegisterDevice(device);
    
    // Device was registered, now process the CompleteDeviceRegistration
    // that was queued from RegisterDevice
    mockEventBus.ClearEvents();
    mockMqttClient.ClearAll();

    // Process one discovery cycle through MqttConnected 
    BridgeEvent connEvent{};
    connEvent.Type = BridgeEvent::Type::MqttConnected;
    processor.Process(connEvent);

    // Act - Process the discovery publish that was queued
    BridgeEvent nextEvent{};
    nextEvent.Type = BridgeEvent::Type::PublishNextDiscovery;
    processor.Process(nextEvent);

    // Assert - Check that discovery was published
    TEST_ASSERT_TRUE(mockMqttClient.HasPublicationWithTopic(
        "homeassistant/switch/device-1/config"));
    TEST_ASSERT_TRUE(mockMqttClient.HasPublicationWithTopic(
        "domo/dev/device-1/state"));
}

// Test: Process MqttData routes command to correct device
void Processor_ProcessMqttData_RoutesCommandToDevice()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device = std::make_shared<MockDevice>("my-switch");
    processor.RegisterDevice(device);

    BridgeEvent dataEvent{};
    dataEvent.Type = BridgeEvent::Type::MqttData;
    std::strcpy(dataEvent.Topic, "domo/dev/my-switch/set");
    dataEvent.TopicLength = 22;
    std::strcpy(dataEvent.Payload, "ON");
    dataEvent.PayloadLength = 2;

    // Act
    processor.Process(dataEvent);

    // Assert
    TEST_ASSERT_EQUAL(1, device->GetProcessCommandCount());
    TEST_ASSERT_EQUAL_STRING("set", device->GetLastSubtopic().c_str());
    TEST_ASSERT_EQUAL_STRING("ON", device->GetLastCommand().c_str());
}

// Test: Process MqttData ignores messages for wrong prefix
void Processor_ProcessMqttData_IgnoresWrongPrefix()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device = std::make_shared<MockDevice>("my-switch");
    processor.RegisterDevice(device);

    BridgeEvent dataEvent{};
    dataEvent.Type = BridgeEvent::Type::MqttData;
    std::strcpy(dataEvent.Topic, "wrong/prefix/my-switch/set");
    dataEvent.TopicLength = 26;
    std::strcpy(dataEvent.Payload, "ON");
    dataEvent.PayloadLength = 2;

    // Act
    processor.Process(dataEvent);

    // Assert
    TEST_ASSERT_EQUAL(0, device->GetProcessCommandCount());
}

// Test: Process MqttData ignores messages without subtopic
void Processor_ProcessMqttData_IgnoresNoSubtopic()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device = std::make_shared<MockDevice>("my-switch");
    processor.RegisterDevice(device);

    BridgeEvent dataEvent{};
    dataEvent.Type = BridgeEvent::Type::MqttData;
    std::strcpy(dataEvent.Topic, "domo/dev/my-switch");
    dataEvent.TopicLength = 18;

    // Act
    processor.Process(dataEvent);

    // Assert
    TEST_ASSERT_EQUAL(0, device->GetProcessCommandCount());
}

// Test: Process PublishState publishes to MQTT
void Processor_ProcessPublishState_PublishesToMqtt()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    mockMqttClient.ClearAll();

    BridgeEvent stateEvent{};
    stateEvent.Type = BridgeEvent::Type::PublishState;
    std::strcpy(stateEvent.Topic, "domo/dev/lamp/state");
    stateEvent.TopicLength = 19;
    std::strcpy(stateEvent.Payload, "OFF");
    stateEvent.PayloadLength = 3;
    stateEvent.Retain = true;

    // Act
    processor.Process(stateEvent);

    // Assert
    TEST_ASSERT_EQUAL(1, mockMqttClient.GetPublicationCount());
    const auto& pub = mockMqttClient.GetPublicationAt(0);
    TEST_ASSERT_EQUAL_STRING("domo/dev/lamp/state", pub.topic.c_str());
    TEST_ASSERT_EQUAL_STRING("OFF", pub.payload.c_str());
    TEST_ASSERT_TRUE(pub.retain);
}

// Test: Process UnregisterDevice removes device and publishes removal
void Processor_ProcessUnregisterDevice_RemovesDeviceAndPublishes()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device = std::make_shared<MockDevice>("old-device");
    processor.RegisterDevice(device);
    mockEventBus.ClearEvents();
    mockMqttClient.ClearAll();

    BridgeEvent unreg{};
    unreg.Type = BridgeEvent::Type::UnregisterDevice;
    std::strcpy(unreg.Id, "old-device");
    unreg.IdLength = 10;

    // Act
    processor.Process(unreg);

    // Assert
    // Should have published device removal (empty payload, retain=true)
    TEST_ASSERT_TRUE(mockMqttClient.GetPublicationCount() >= 1);
    const auto* removal = mockMqttClient.FindPublicationByTopic(
        "homeassistant/switch/old-device/config");
    TEST_ASSERT_NOT_NULL(removal);
    TEST_ASSERT_EQUAL_STRING("", removal->payload.c_str());
    TEST_ASSERT_TRUE(removal->retain);
}

// Test: Process PublishNextDiscovery publishes discovery for first device
void Processor_ProcessPublishNextDiscovery_PublishesDiscovery()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device = std::make_shared<MockDevice>("lamp-1");
    processor.RegisterDevice(device);

    BridgeEvent connEvent{};
    connEvent.Type = BridgeEvent::Type::MqttConnected;
    processor.Process(connEvent);
    mockMqttClient.ClearAll();

    BridgeEvent nextEvent{};
    nextEvent.Type = BridgeEvent::Type::PublishNextDiscovery;

    // Act
    processor.Process(nextEvent);

    // Assert
    TEST_ASSERT_TRUE(mockMqttClient.HasPublicationWithTopic(
        "homeassistant/switch/lamp-1/config"));
}

// Test: Process PublishNextDiscovery with multiple devices enqueues more events
void Processor_ProcessPublishNextDiscovery_Multiple_EnqueuesMoreEvents()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device1 = std::make_shared<MockDevice>("device-1");
    auto device2 = std::make_shared<MockDevice>("device-2");
    processor.RegisterDevice(device1);
    processor.RegisterDevice(device2);

    BridgeEvent connEvent{};
    connEvent.Type = BridgeEvent::Type::MqttConnected;
    processor.Process(connEvent);
    mockEventBus.ClearEvents();

    BridgeEvent nextEvent{};
    nextEvent.Type = BridgeEvent::Type::PublishNextDiscovery;

    // Act
    processor.Process(nextEvent);

    // Assert
    // Should enqueue another PublishNextDiscovery for the second device
    TEST_ASSERT_TRUE(mockEventBus.HasEventWithTypeValue(
        static_cast<int>(BridgeEvent::Type::PublishNextDiscovery)));
}

// Test: Multiple device registrations
void Processor_MultipleDeviceRegistrations_AllPublish()
{
    // Arrange
    Processor processor(mockMqttClient, mockEventBus);
    auto device1 = std::make_shared<MockDevice>("switch-1");
    auto device2 = std::make_shared<MockDevice>("switch-2");
    processor.RegisterDevice(device1);
    processor.RegisterDevice(device2);

    mockEventBus.ClearEvents();
    mockMqttClient.ClearAll();

    // Connect to trigger discovery
    BridgeEvent connEvent{};
    connEvent.Type = BridgeEvent::Type::MqttConnected;
    processor.Process(connEvent);

    // Process first discovery
    BridgeEvent nextEvent{};
    nextEvent.Type = BridgeEvent::Type::PublishNextDiscovery;
    processor.Process(nextEvent);

    // Process second discovery
    processor.Process(nextEvent);

    // Assert
    TEST_ASSERT_TRUE(mockMqttClient.HasPublicationWithTopic(
        "homeassistant/switch/switch-1/config"));
    TEST_ASSERT_TRUE(mockMqttClient.HasPublicationWithTopic(
        "homeassistant/switch/switch-2/config"));
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(Processor_RegisterDevice_EnqueuesCompleteDeviceRegistrationEvent);
    RUN_TEST(Processor_UnregisterDevice_EnqueuesUnregisterDeviceEvent);
    RUN_TEST(Processor_ProcessMqttConnected_SubscribesToTopic);
    RUN_TEST(Processor_ProcessMqttConnected_WithRegisteredDevices_EnqueuesDiscovery);
    RUN_TEST(Processor_ProcessCompleteDeviceRegistration_PublishesDiscoveryAndState);
    RUN_TEST(Processor_ProcessMqttData_RoutesCommandToDevice);
    RUN_TEST(Processor_ProcessMqttData_IgnoresWrongPrefix);
    RUN_TEST(Processor_ProcessMqttData_IgnoresNoSubtopic);
    RUN_TEST(Processor_ProcessPublishState_PublishesToMqtt);
    RUN_TEST(Processor_ProcessUnregisterDevice_RemovesDeviceAndPublishes);
    RUN_TEST(Processor_ProcessPublishNextDiscovery_PublishesDiscovery);
    RUN_TEST(Processor_ProcessPublishNextDiscovery_Multiple_EnqueuesMoreEvents);
    RUN_TEST(Processor_MultipleDeviceRegistrations_AllPublish);

    return UNITY_END();
}
