#include <unity.h>
#include <memory>

#include <Modules/DimmerModule.h>
#include <DimmerControlValue.h>
#include "../../Mocks/MockBus.h"
#include <KnownModuleIdentifiers.h>

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void DimmerModule_Constructor_InitializesCorrectly()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t initialData = 0;
    
    // Act
    DimmerModule module(bus, address, initialData);
    
    // Assert
    TEST_ASSERT_EQUAL(ModuleType::Dimmer, module.GetType());
    TEST_ASSERT_EQUAL(address, module.GetAddress());
    
    auto dimmerPins = module.GetDimmerControlPins();
    TEST_ASSERT_EQUAL(12, dimmerPins.size()); // 12 dimmer channels as defined in the constructor
    
    // Verify each pin is initially set to 0% with 0 fade time
    for (const auto& weakPin : dimmerPins) {
        auto pin = weakPin.lock();
        TEST_ASSERT_NOT_NULL(pin.get());
        TEST_ASSERT_EQUAL(0, pin->GetState().GetPercentage());
        TEST_ASSERT_EQUAL(0, pin->GetState().GetFadeTimeInSeconds());
    }
}

void DimmerModule_Process_SuccessfulPoll()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t initialData = 0;
    
    // Setup mock responses
    ScanResponse pollResponse = {
        .Success = true,
        .RespondedWithTypeAndData = false
    };
    bus.QueueResponse(pollResponse);
    
    DimmerModule module(bus, address, initialData);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_FALSE(response.RaisePriority);
    TEST_ASSERT_TRUE(bus.PollCalled);
    TEST_ASSERT_FALSE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(address, bus.LastPolledAddress);
}

void DimmerModule_Process_FailedPoll()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t initialData = 0;
    
    // Setup mock responses with a failed poll
    ScanResponse pollResponse = {
        .Success = false
    };
    bus.QueueResponse(pollResponse);
    
    DimmerModule module(bus, address, initialData);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_FALSE(response.Success);
    TEST_ASSERT_TRUE(bus.PollCalled);
    TEST_ASSERT_FALSE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(address, bus.LastPolledAddress);
}

void DimmerModule_UpdateChannel_SendsCorrectCommands()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t initialData = 0;
    
    // Setup mock responses for the two Exchange calls
    ScanResponse response1 = {
        .Success = true
    };
    ScanResponse response2 = {
        .Success = true
    };
    bus.QueueResponse(response1);
    bus.QueueResponse(response2);
    
    DimmerModule module(bus, address, initialData);
    
    // Get a handle to the first dimmer channel pin
    auto dimmerPins = module.GetDimmerControlPins();
    auto pin = dimmerPins[0].lock();
    
    // Act - set the dimmer to 75% with a fade time of 5 seconds
    DimmerControlValue value(75, 5);
    pin->SetState(value);
    
    // Assert
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
    
    // The last exchange should have been for setting the percentage (CMD1 = 0x01)
    // with channel 1 (index 0 + 1) << 4 = 0x10 and 75% << 8 = 0x4B00
    TEST_ASSERT_EQUAL(0x01 | (75 << 8) | (1 << 4), bus.LastExchangeData);
}

void DimmerModule_ToString_ReturnsCorrectString()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x15;
    const uint16_t initialData = 0;
    
    DimmerModule module(bus, address, initialData);
    
    // Act
    std::string result = module.ToString();
    
    // Assert
    std::string expected = std::string(KnownModuleIdentifiers::Dimmer) + " 21 12"; // address in decimal is 21
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), result.c_str());
}

void DimmerModule_UpdateMultipleChannels_SendsCorrectCommands()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t initialData = 0;
    
    // Setup mock responses for the Exchange calls (4 calls for 2 channels)
    for (int i = 0; i < 4; i++) {
        bus.QueueResponse({ .Success = true });
    }
    
    DimmerModule module(bus, address, initialData);
    
    // Get handles to multiple dimmer channel pins
    auto dimmerPins = module.GetDimmerControlPins();
    auto pin0 = dimmerPins[0].lock();
    auto pin5 = dimmerPins[5].lock();
    
    // Clear mock bus call flags
    bus.ClearCalls();
    
    // Act - set the dimmers to different values
    pin0->SetState(DimmerControlValue(50, 2)); // 50% with 2 seconds fade
    pin5->SetState(DimmerControlValue(100, 10)); // 100% with 10 seconds fade
    
    // Assert - Check that the correct commands were sent for both channels
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
    
    // The last exchange should have been for setting the percentage for channel 6 (index 5 + 1)
    TEST_ASSERT_EQUAL(0x01 | (100 << 8) | (6 << 4), bus.LastExchangeData);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(DimmerModule_Constructor_InitializesCorrectly);
    RUN_TEST(DimmerModule_Process_SuccessfulPoll);
    RUN_TEST(DimmerModule_Process_FailedPoll);
    RUN_TEST(DimmerModule_UpdateChannel_SendsCorrectCommands);
    RUN_TEST(DimmerModule_ToString_ReturnsCorrectString);
    RUN_TEST(DimmerModule_UpdateMultipleChannels_SendsCorrectCommands);
    
    return UNITY_END();
}
