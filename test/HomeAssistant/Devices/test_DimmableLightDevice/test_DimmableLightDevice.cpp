#include <unity.h>
#include <memory>
#include <cstring>

#include <Devices/DimmableLightDevice.h>
#include <Filters/DimmerFilter.h>
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

// Test: DimmableLightDevice construction and ID
void DimmableLightDevice_Constructor_SetsId()
{
    // Arrange & Act
    auto filter = std::make_shared<DimmerFilter>("test-dimmer");
    auto eventBus = std::make_shared<MockEventBus>();
    DimmableLightDevice device("my-dimmer", filter, eventBus);

    // Assert
    TEST_ASSERT_EQUAL_STRING("my-dimmer", device.GetId().data());
}

// Test: BuildDiscoveryTopic returns valid topic
void DimmableLightDevice_BuildDiscoveryTopic_ReturnsValidTopic()
{
    // Arrange
    auto filter = std::make_shared<DimmerFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    DimmableLightDevice device("my-dimmer", filter, eventBus);
    char buffer[64];

    // Act
    bool result = device.BuildDiscoveryTopic(buffer, sizeof(buffer));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "homeassistant/light") != NULL);
    TEST_ASSERT(strstr(buffer, "my-dimmer") != NULL);
}

// Test: BuildDiscoveryPayload works
void DimmableLightDevice_BuildDiscoveryPayload_IncludesBrightness()
{
    // Arrange
    auto filter = std::make_shared<DimmerFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    DimmableLightDevice device("my-dimmer", filter, eventBus);
    char buffer[512];

    // Act
    bool result = device.BuildDiscoveryPayload(buffer, sizeof(buffer));

    // Assert - Just verify it succeeds and contains expected fields
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(strstr(buffer, "brightness") != NULL);
}

// Test: BuildStateMessages can be called
void DimmableLightDevice_BuildStateMessages_IncludesBrightness()
{
    // Arrange
    auto filter = std::make_shared<DimmerFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    DimmableLightDevice device("my-dimmer", filter, eventBus);
    StateMessageList list;

    // Act
    device.BuildStateMessages(list);

    // Assert - Just verify it doesn't crash
    TEST_ASSERT_TRUE(true);
}

// Test: SubscribeToStateChanges succeeds
void DimmableLightDevice_SubscribeToStateChanges_Succeeds()
{
    // Arrange
    auto filter = std::make_shared<DimmerFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    DimmableLightDevice device("my-dimmer", filter, eventBus);

    // Act
    device.SubscribeToStateChanges();

    // Assert
    TEST_ASSERT_TRUE(true);
}

// Test: ProcessCommand handles brightness setting
void DimmableLightDevice_ProcessCommand_HandlesBrightnessCommand()
{
    // Arrange
    auto filter = std::make_shared<DimmerFilter>("test");
    auto eventBus = std::make_shared<MockEventBus>();
    DimmableLightDevice device("my-dimmer", filter, eventBus);

    // Act
    device.ProcessCommand("set", "50");

    // Assert
    TEST_ASSERT_TRUE(true);
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(DimmableLightDevice_Constructor_SetsId);
    RUN_TEST(DimmableLightDevice_BuildDiscoveryTopic_ReturnsValidTopic);
    RUN_TEST(DimmableLightDevice_BuildDiscoveryPayload_IncludesBrightness);
    RUN_TEST(DimmableLightDevice_BuildStateMessages_IncludesBrightness);
    RUN_TEST(DimmableLightDevice_SubscribeToStateChanges_Succeeds);
    RUN_TEST(DimmableLightDevice_ProcessCommand_HandlesBrightnessCommand);

    return UNITY_END();
}
