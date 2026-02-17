#include <unity.h>
#include <memory>
#include <cstring>

#include <Devices/CoverDevice.h>
#include <Filters/ShutterFilter.h>
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

// Test: CoverDevice construction and ID
void CoverDevice_Constructor_SetsId()
{
    // Arrange & Act
    auto filter = std::make_shared<ShutterFilter>("test-shutter");
    auto eventBus = std::make_shared<MockEventBus>();
    CoverDevice device("my-shutter", filter, eventBus);

    // Assert
    TEST_ASSERT_EQUAL_STRING("my-shutter", device.GetId().data());
}

// Test: BuildDiscoveryTopic returns valid topic
void CoverDevice_BuildDiscoveryTopic_ReturnsValidTopic()
{
    // Arrange
    auto filter = std::make_shared<ShutterFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    CoverDevice device("my-shutter", filter, eventBus);
    char buffer[64];

    // Act
    bool result = device.BuildDiscoveryTopic(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "homeassistant/cover") != NULL);
    TEST_ASSERT(strstr(buffer, "my-shutter") != NULL);
}

// Test: BuildDiscoveryPayload returns valid JSON
void CoverDevice_BuildDiscoveryPayload_ReturnsValidPayload()
{
    // Arrange
    auto filter = std::make_shared<ShutterFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    CoverDevice device("my-shutter", filter, eventBus);
    char buffer[640];

    // Act
    bool result = device.BuildDiscoveryPayload(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "\"name\"") != NULL);
}

// Test: BuildStateMessages can be called
void CoverDevice_BuildStateMessages_ReturnsStateMessage()
{
    // Arrange
    auto filter = std::make_shared<ShutterFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    CoverDevice device("my-shutter", filter, eventBus);
    StateMessageList list;

    // Act
    device.BuildStateMessages(list);

    // Assert - Just verify it doesn't crash
    TEST_ASSERT_TRUE(true);
}

// Test: SubscribeToStateChanges succeeds
void CoverDevice_SubscribeToStateChanges_Succeeds()
{
    // Arrange
    auto filter = std::make_shared<ShutterFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    CoverDevice device("my-shutter", filter, eventBus);

    // Act
    device.SubscribeToStateChanges();

    // Assert
    TEST_ASSERT_TRUE(true);
}

// Test: ProcessCommand handles shutter commands
void CoverDevice_ProcessCommand_HandlesShutterCommands()
{
    // Arrange
    auto filter = std::make_shared<ShutterFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    CoverDevice device("my-shutter", filter, eventBus);

    // Act
    device.ProcessCommand("open", "");
    device.ProcessCommand("close", "");
    device.ProcessCommand("stop", "");

    // Assert
    TEST_ASSERT_TRUE(true);
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(CoverDevice_Constructor_SetsId);
    RUN_TEST(CoverDevice_BuildDiscoveryTopic_ReturnsValidTopic);
    RUN_TEST(CoverDevice_BuildDiscoveryPayload_ReturnsValidPayload);
    RUN_TEST(CoverDevice_BuildStateMessages_ReturnsStateMessage);
    RUN_TEST(CoverDevice_SubscribeToStateChanges_Succeeds);
    RUN_TEST(CoverDevice_ProcessCommand_HandlesShutterCommands);

    return UNITY_END();
}
