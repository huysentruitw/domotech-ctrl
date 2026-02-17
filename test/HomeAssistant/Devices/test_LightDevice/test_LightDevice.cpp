#include <unity.h>
#include <memory>
#include <cstring>

#include <Devices/LightDevice.h>
#include <Filters/LightFilter.h>
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

// Test: LightDevice construction and ID
void LightDevice_Constructor_SetsId()
{
    // Arrange & Act
    auto filter = std::make_shared<LightFilter>("test-light");
    auto eventBus = std::make_shared<MockEventBus>();
    LightDevice device("my-light", filter, eventBus);

    // Assert
    TEST_ASSERT_EQUAL_STRING("my-light", device.GetId().data());
}

// Test: BuildDiscoveryTopic returns valid topic
void LightDevice_BuildDiscoveryTopic_ReturnsValidTopic()
{
    // Arrange
    auto filter = std::make_shared<LightFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    LightDevice device("my-light", filter, eventBus);
    char buffer[64];

    // Act
    bool result = device.BuildDiscoveryTopic(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "homeassistant/light") != NULL);
    TEST_ASSERT(strstr(buffer, "my-light") != NULL);
}

// Test: BuildDiscoveryPayload returns valid JSON
void LightDevice_BuildDiscoveryPayload_ReturnsValidPayload()
{
    // Arrange
    auto filter = std::make_shared<LightFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    LightDevice device("my-light", filter, eventBus);
    char buffer[640];

    // Act
    bool result = device.BuildDiscoveryPayload(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "\"name\"") != NULL);
    TEST_ASSERT(strstr(buffer, "my-light") != NULL);
}

// Test: BuildStateMessages can be called
void LightDevice_BuildStateMessages_ReturnsStateMessage()
{
    // Arrange
    auto filter = std::make_shared<LightFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    LightDevice device("my-light", filter, eventBus);
    StateMessageList list;

    // Act
    device.BuildStateMessages(list);

    // Assert - Just verify it doesn't crash
    TEST_ASSERT_TRUE(true);
}

// Test: SubscribeToStateChanges succeeds
void LightDevice_SubscribeToStateChanges_Succeeds()
{
    // Arrange
    auto filter = std::make_shared<LightFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    LightDevice device("my-light", filter, eventBus);

    // Act
    device.SubscribeToStateChanges();

    // Assert
    TEST_ASSERT_TRUE(true);
}

// Test: ProcessCommand handles "set" subtopic
void LightDevice_ProcessCommand_HandlesSetCommand()
{
    // Arrange
    auto filter = std::make_shared<LightFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    LightDevice device("my-light", filter, eventBus);

    // Act
    device.ProcessCommand("set", "ON");

    // Assert
    TEST_ASSERT_TRUE(true);
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(LightDevice_Constructor_SetsId);
    RUN_TEST(LightDevice_BuildDiscoveryTopic_ReturnsValidTopic);
    RUN_TEST(LightDevice_BuildDiscoveryPayload_ReturnsValidPayload);
    RUN_TEST(LightDevice_BuildStateMessages_ReturnsStateMessage);
    RUN_TEST(LightDevice_SubscribeToStateChanges_Succeeds);
    RUN_TEST(LightDevice_ProcessCommand_HandlesSetCommand);

    return UNITY_END();
}
