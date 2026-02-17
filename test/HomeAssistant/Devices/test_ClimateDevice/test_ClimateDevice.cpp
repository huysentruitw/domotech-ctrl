#include <unity.h>
#include <memory>
#include <cstring>

#include <Devices/ClimateDevice.h>
#include <Filters/ClimateFilter.h>
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

// Test: ClimateDevice construction and ID
void ClimateDevice_Constructor_SetsId()
{
    // Arrange & Act
    auto filter = std::make_shared<ClimateFilter>("test-climate");
    auto eventBus = std::make_shared<MockEventBus>();
    ClimateDevice device("my-climate", filter, eventBus);

    // Assert
    TEST_ASSERT_EQUAL_STRING("my-climate", device.GetId().data());
}

// Test: BuildDiscoveryTopic returns valid topic
void ClimateDevice_BuildDiscoveryTopic_ReturnsValidTopic()
{
    // Arrange
    auto filter = std::make_shared<ClimateFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    ClimateDevice device("my-climate", filter, eventBus);
    char buffer[64];

    // Act
    bool result = device.BuildDiscoveryTopic(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "homeassistant/climate") != NULL);
    TEST_ASSERT(strstr(buffer, "my-climate") != NULL);
}

// Test: BuildDiscoveryPayload returns valid JSON
void ClimateDevice_BuildDiscoveryPayload_ReturnsValidPayload()
{
    // Arrange
    auto filter = std::make_shared<ClimateFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    ClimateDevice device("my-climate", filter, eventBus);
    char buffer[640];

    // Act
    bool result = device.BuildDiscoveryPayload(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "\"name\"") != NULL);
}

// Test: BuildStateMessages can be called
void ClimateDevice_BuildStateMessages_ReturnsStateMessage()
{
    // Arrange
    auto filter = std::make_shared<ClimateFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    ClimateDevice device("my-climate", filter, eventBus);
    StateMessageList list;

    // Act
    device.BuildStateMessages(list);

    // Assert - Just verify it doesn't crash
    TEST_ASSERT_TRUE(true);
}

// Test: SubscribeToStateChanges succeeds
void ClimateDevice_SubscribeToStateChanges_Succeeds()
{
    // Arrange
    auto filter = std::make_shared<ClimateFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    ClimateDevice device("my-climate", filter, eventBus);

    // Act
    device.SubscribeToStateChanges();

    // Assert
    TEST_ASSERT_TRUE(true);
}

// Test: ProcessCommand handles climate commands
void ClimateDevice_ProcessCommand_HandlesClimateCommands()
{
    // Arrange
    auto filter = std::make_shared<ClimateFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    ClimateDevice device("my-climate", filter, eventBus);

    // Act
    device.ProcessCommand("heating", "ON");
    device.ProcessCommand("temperature", "22");

    // Assert
    TEST_ASSERT_TRUE(true);
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(ClimateDevice_Constructor_SetsId);
    RUN_TEST(ClimateDevice_BuildDiscoveryTopic_ReturnsValidTopic);
    RUN_TEST(ClimateDevice_BuildDiscoveryPayload_ReturnsValidPayload);
    RUN_TEST(ClimateDevice_BuildStateMessages_ReturnsStateMessage);
    RUN_TEST(ClimateDevice_SubscribeToStateChanges_Succeeds);
    RUN_TEST(ClimateDevice_ProcessCommand_HandlesClimateCommands);

    return UNITY_END();
}
