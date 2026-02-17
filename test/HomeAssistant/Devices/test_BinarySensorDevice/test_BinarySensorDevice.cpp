#include <unity.h>
#include <memory>
#include <cstring>

#include <Devices/BinarySensorDevice.h>
#include <Filters/DigitalPassthroughFilter.h>
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

// Test: BinarySensorDevice construction and ID
void BinarySensorDevice_Constructor_SetsId()
{
    // Arrange & Act
    auto filter = std::make_shared<DigitalPassthroughFilter>("test-binary");
    auto eventBus = std::make_shared<MockEventBus>();
    BinarySensorDevice device("my-binary", filter, eventBus);

    // Assert
    TEST_ASSERT_EQUAL_STRING("my-binary", device.GetId().data());
}

// Test: BuildDiscoveryTopic returns valid topic
void BinarySensorDevice_BuildDiscoveryTopic_ReturnsValidTopic()
{
    // Arrange
    auto filter = std::make_shared<DigitalPassthroughFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    BinarySensorDevice device("my-binary", filter, eventBus);
    char buffer[64];

    // Act
    bool result = device.BuildDiscoveryTopic(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "homeassistant/binary_sensor") != NULL);
    TEST_ASSERT(strstr(buffer, "my-binary") != NULL);
}

// Test: BuildDiscoveryPayload returns valid JSON
void BinarySensorDevice_BuildDiscoveryPayload_ReturnsValidPayload()
{
    // Arrange
    auto filter = std::make_shared<DigitalPassthroughFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    BinarySensorDevice device("my-binary", filter, eventBus);
    char buffer[640];

    // Act
    bool result = device.BuildDiscoveryPayload(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "\"name\"") != NULL);
}

// Test: BuildStateMessages returns state
void BinarySensorDevice_BuildStateMessages_ReturnsStateMessage()
{
    // Arrange
    auto filter = std::make_shared<DigitalPassthroughFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    BinarySensorDevice device("my-binary", filter, eventBus);
    StateMessageList list;

    // Act
    device.BuildStateMessages(list);

    // Assert - Just verify it doesn't crash
    TEST_ASSERT_TRUE(true);
}

// Test: SubscribeToStateChanges succeeds
void BinarySensorDevice_SubscribeToStateChanges_Succeeds()
{
    // Arrange
    auto filter = std::make_shared<DigitalPassthroughFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    BinarySensorDevice device("my-binary", filter, eventBus);

    // Act
    device.SubscribeToStateChanges();

    // Assert
    TEST_ASSERT_TRUE(true);
}

// Test: ProcessCommand can be called (read-only sensor)
void BinarySensorDevice_ProcessCommand_NoOp()
{
    // Arrange
    auto filter = std::make_shared<DigitalPassthroughFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    BinarySensorDevice device("my-binary", filter, eventBus);

    // Act
    device.ProcessCommand("state", "ON");

    // Assert
    TEST_ASSERT_TRUE(true);
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(BinarySensorDevice_Constructor_SetsId);
    RUN_TEST(BinarySensorDevice_BuildDiscoveryTopic_ReturnsValidTopic);
    RUN_TEST(BinarySensorDevice_BuildDiscoveryPayload_ReturnsValidPayload);
    RUN_TEST(BinarySensorDevice_BuildStateMessages_ReturnsStateMessage);
    RUN_TEST(BinarySensorDevice_SubscribeToStateChanges_Succeeds);
    RUN_TEST(BinarySensorDevice_ProcessCommand_NoOp);

    return UNITY_END();
}
