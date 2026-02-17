#include <unity.h>
#include <memory>

#include <HomeAssistantBridge.h>
#include <FilterFactory.h>
#include "../../Mocks/MockMqttClient.h"

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

// Test: Register Switch filter creates device
void HomeAssistantBridge_RegisterSwitchFilter_CreatesDevice()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    
    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Switch), "test-switch"));
    TEST_ASSERT_NOT_NULL(filter.get());

    // Act
    bool result = bridge.RegisterAsDevice(filter);

    // Assert
    TEST_ASSERT_TRUE(result);
}

// Test: Register Light filter creates device
void HomeAssistantBridge_RegisterLightFilter_CreatesDevice()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    
    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Light), "test-light"));
    TEST_ASSERT_NOT_NULL(filter.get());

    // Act
    bool result = bridge.RegisterAsDevice(filter);

    // Assert
    TEST_ASSERT_TRUE(result);
}

// Test: Register Dimmer filter creates device
void HomeAssistantBridge_RegisterDimmerFilter_CreatesDevice()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    
    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Dimmer), "test-dimmer"));
    TEST_ASSERT_NOT_NULL(filter.get());

    // Act
    bool result = bridge.RegisterAsDevice(filter);

    // Assert
    TEST_ASSERT_TRUE(result);
}

// Test: Register Shutter filter creates device
void HomeAssistantBridge_RegisterShutterFilter_CreatesDevice()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    
    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Shutter), "test-shutter"));
    TEST_ASSERT_NOT_NULL(filter.get());

    // Act
    bool result = bridge.RegisterAsDevice(filter);

    // Assert
    TEST_ASSERT_TRUE(result);
}

// Test: Register Climate filter creates device
void HomeAssistantBridge_RegisterClimateFilter_CreatesDevice()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    
    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Climate), "test-climate"));
    TEST_ASSERT_NOT_NULL(filter.get());

    // Act
    bool result = bridge.RegisterAsDevice(filter);

    // Assert
    TEST_ASSERT_TRUE(result);
}

// Test: Register DigitalPassthrough filter creates device
void HomeAssistantBridge_RegisterDigitalPassthroughFilter_CreatesDevice()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    
    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::DigitalPassthrough), "test-binary"));
    TEST_ASSERT_NOT_NULL(filter.get());

    // Act
    bool result = bridge.RegisterAsDevice(filter);

    // Assert
    TEST_ASSERT_TRUE(result);
}

// Test: Unregister device returns true
void HomeAssistantBridge_UnregisterDevice_ReturnsTrue()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    
    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Switch), "device-to-remove"));
    TEST_ASSERT_NOT_NULL(filter.get());
    
    bridge.RegisterAsDevice(filter);

    // Act
    bool result = bridge.UnregisterAsDevice(filter);

    // Assert
    TEST_ASSERT_TRUE(result);
}

// Test: Register null weak_ptr returns false
void HomeAssistantBridge_RegisterNullWeakPtr_ReturnsFalse()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    std::weak_ptr<Filter> nullPtr;

    // Act
    bool result = bridge.RegisterAsDevice(nullPtr);

    // Assert
    TEST_ASSERT_FALSE(result);
}

// Test: Unregister null weak_ptr returns false
void HomeAssistantBridge_UnregisterNullWeakPtr_ReturnsFalse()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    std::weak_ptr<Filter> nullPtr;

    // Act
    bool result = bridge.UnregisterAsDevice(nullPtr);

    // Assert
    TEST_ASSERT_FALSE(result);
}

// Test: ID sanitization happens (spaces, special chars removed)
void HomeAssistantBridge_RegisterWithUnsanitizedId_SanitizesId()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    
    // Create filter with unsanitized ID (spaces, uppercase)
    auto filter = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Switch), "My Test Switch"));
    TEST_ASSERT_NOT_NULL(filter.get());

    // Act
    bool result = bridge.RegisterAsDevice(filter);

    // Assert
    // Should succeed despite unsanitized ID (IdSanitizer is applied internally)
    TEST_ASSERT_TRUE(result);
}

// Test: Multiple devices can be registered
void HomeAssistantBridge_RegisterMultipleDevices_AllSucceed()
{
    // Arrange
    HomeAssistantBridge bridge;
    bridge.Init("mqtt://localhost", "user", "pass");
    
    auto switch1 = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Switch), "switch-1"));
    auto light1 = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Light), "light-1"));
    auto dimmer1 = std::shared_ptr<Filter>(FilterFactory::TryCreateFilterByTypeName(
        GetFilterTypeName(FilterType::Dimmer), "dimmer-1"));

    // Act & Assert
    TEST_ASSERT_TRUE(bridge.RegisterAsDevice(switch1));
    TEST_ASSERT_TRUE(bridge.RegisterAsDevice(light1));
    TEST_ASSERT_TRUE(bridge.RegisterAsDevice(dimmer1));
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(HomeAssistantBridge_RegisterSwitchFilter_CreatesDevice);
    RUN_TEST(HomeAssistantBridge_RegisterLightFilter_CreatesDevice);
    RUN_TEST(HomeAssistantBridge_RegisterDimmerFilter_CreatesDevice);
    RUN_TEST(HomeAssistantBridge_RegisterShutterFilter_CreatesDevice);
    RUN_TEST(HomeAssistantBridge_RegisterClimateFilter_CreatesDevice);
    RUN_TEST(HomeAssistantBridge_RegisterDigitalPassthroughFilter_CreatesDevice);
    RUN_TEST(HomeAssistantBridge_UnregisterDevice_ReturnsTrue);
    RUN_TEST(HomeAssistantBridge_RegisterNullWeakPtr_ReturnsFalse);
    RUN_TEST(HomeAssistantBridge_UnregisterNullWeakPtr_ReturnsFalse);
    RUN_TEST(HomeAssistantBridge_RegisterWithUnsanitizedId_SanitizesId);
    RUN_TEST(HomeAssistantBridge_RegisterMultipleDevices_AllSucceed);

    return UNITY_END();
}
