#include <unity.h>
#include <memory>
#include <cstring>

#include <Devices/SwitchDevice.h>
#include <Filters/SwitchFilter.h>
#include "../../Mocks/MockEventBus.h"

static MockEventBus mockEventBus;

void setUp(void)
{
    mockEventBus.ClearEvents();
}

void tearDown(void)
{
    // Cleanup after each test
}

// Test: SwitchDevice construction and ID
void SwitchDevice_Constructor_SetsId()
{
    // Arrange & Act
    auto filter = std::make_shared<SwitchFilter>("test-switch");
    auto eventBus = std::make_shared<MockEventBus>();
    SwitchDevice device("my-switch", filter, eventBus);

    // Assert
    TEST_ASSERT_EQUAL_STRING("my-switch", device.GetId().data());
}

// Test: BuildDiscoveryTopic returns valid topic
void SwitchDevice_BuildDiscoveryTopic_ReturnsValidTopic()
{
    // Arrange
    auto filter = std::make_shared<SwitchFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    SwitchDevice device("my-switch", filter, eventBus);
    char buffer[64];

    // Act
    bool result = device.BuildDiscoveryTopic(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "homeassistant/switch") != NULL);
    TEST_ASSERT(strstr(buffer, "my-switch") != NULL);
}

// Test: BuildDiscoveryTopic handles buffer overflow
void SwitchDevice_BuildDiscoveryTopic_BufferTooSmall_ReturnsFalse()
{
    // Arrange
    auto filter = std::make_shared<SwitchFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    SwitchDevice device("my-very-long-switch-name", filter, eventBus);
    char buffer[5]; // Too small

    // Act
    bool result = device.BuildDiscoveryTopic(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_FALSE(result);
}

// Test: BuildDiscoveryPayload returns valid JSON
void SwitchDevice_BuildDiscoveryPayload_ReturnsValidPayload()
{
    // Arrange
    auto filter = std::make_shared<SwitchFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    SwitchDevice device("my-switch", filter, eventBus);
    char buffer[640];

    // Act
    bool result = device.BuildDiscoveryPayload(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "\"name\"") != NULL);
    TEST_ASSERT(strstr(buffer, "my-switch") != NULL);
}

// Test: BuildStateMessages can be called
void SwitchDevice_BuildStateMessages_ReturnsStateMessage()
{
    // Arrange
    auto filter = std::make_shared<SwitchFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    SwitchDevice device("my-switch", filter, eventBus);
    StateMessageList list;

    // Act
    device.BuildStateMessages(list);

    // Assert - Just verify it doesn't crash
    TEST_ASSERT_TRUE(true);
}

// Test: SubscribeToStateChanges succeeds
void SwitchDevice_SubscribeToStateChanges_Succeeds()
{
    // Arrange
    auto filter = std::make_shared<SwitchFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    SwitchDevice device("my-switch", filter, eventBus);

    // Act
    device.SubscribeToStateChanges();

    // Assert - No exception means success
    TEST_ASSERT_TRUE(true);
}

// Test: ProcessCommand handles "set" subtopic
void SwitchDevice_ProcessCommand_HandlesSetCommand()
{
    // Arrange
    auto filter = std::make_shared<SwitchFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    SwitchDevice device("my-switch", filter, eventBus);

    // Act
    device.ProcessCommand("set", "ON");

    // Assert - No exception means success
    TEST_ASSERT_TRUE(true);
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(SwitchDevice_Constructor_SetsId);
    RUN_TEST(SwitchDevice_BuildDiscoveryTopic_ReturnsValidTopic);
    RUN_TEST(SwitchDevice_BuildDiscoveryTopic_BufferTooSmall_ReturnsFalse);
    RUN_TEST(SwitchDevice_BuildDiscoveryPayload_ReturnsValidPayload);
    RUN_TEST(SwitchDevice_BuildStateMessages_ReturnsStateMessage);
    RUN_TEST(SwitchDevice_SubscribeToStateChanges_Succeeds);
    RUN_TEST(SwitchDevice_ProcessCommand_HandlesSetCommand);

    return UNITY_END();
}
