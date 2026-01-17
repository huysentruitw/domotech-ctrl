#include <unity.h>
#include <memory>

#include <Modules/PushButtonModule.h>
#include "../../Mocks/MockBus.h"

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void PushButtonModule_CreateFromInitialData_InitializesCorrectly()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t initialData = 0x4000; // 4 in the top 4 bits (4 buttons)
    
    // Act
    auto module = PushButtonModule::CreateFromInitialData(bus, address, initialData);
    
    // Assert
    auto outputPins = module->GetOutputPins();
    TEST_ASSERT_EQUAL(4, outputPins.size());
    
    // Verify each pin is initially set to false
    for (const auto& weakPin : outputPins)
    {
        auto pin = weakPin.lock();
        TEST_ASSERT_NOT_NULL(pin.get());
        TEST_ASSERT_EQUAL(DigitalValue(false), pin->GetStateAs<DigitalValue>());
    }
}

void PushButtonModule_Process_SuccessfulPoll_NoButtonsPressed()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t numberOfButtons = 2;
    
    // Setup mock responses
    ScanResponse pollResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = false
    };
    bus.QueueResponse(pollResponse);
    
    PushButtonModule module(bus, address, numberOfButtons);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_FALSE(response.RaisePriority);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_FALSE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
}

void PushButtonModule_Process_SuccessfulPoll_ButtonsPressed()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t numberOfButtons = 8;
    
    // Setup mock response with buttons pressed (0x0010 is button 0 pressed)
    ScanResponse pollResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0010
    };
    bus.QueueResponse(pollResponse);
    
    PushButtonModule module(bus, address, numberOfButtons);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_TRUE(response.RaisePriority);
    
    // Verify button pin states
    auto outputPins = module.GetOutputPins();
    TEST_ASSERT_EQUAL(DigitalValue(true), outputPins[0].lock()->GetStateAs<DigitalValue>()); // Button 0 should be pressed
    for (size_t i = 1; i < outputPins.size(); i++)
    {
        TEST_ASSERT_EQUAL(DigitalValue(false), outputPins[i].lock()->GetStateAs<DigitalValue>()); // Other buttons should not be pressed
    }
}

void PushButtonModule_Process_ButtonsReleased()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t numberOfButtons = 8;
    
    // First response with buttons pressed
    ScanResponse firstResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0010 // Button 0 pressed
    };
    bus.QueueResponse(firstResponse);
    
    // Second response with no buttons pressed
    ScanResponse secondResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = false
    };
    bus.QueueResponse(secondResponse);
    
    PushButtonModule module(bus, address, numberOfButtons);
    
    // Press the button first
    auto firstProcessResponse = module.Process();
    TEST_ASSERT_TRUE(firstProcessResponse.Success);
    TEST_ASSERT_TRUE(firstProcessResponse.RaisePriority);
    
    // Act - Process again to release buttons
    bus.ClearCalls();
    auto secondProcessResponse = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(secondProcessResponse.Success);
    TEST_ASSERT_FALSE(secondProcessResponse.RaisePriority);
    
    // Verify all buttons are released
    auto outputPins = module.GetOutputPins();
    for (const auto& weakPin : outputPins)
    {
        TEST_ASSERT_EQUAL(DigitalValue(false), weakPin.lock()->GetStateAs<DigitalValue>());
    }
}

void PushButtonModule_Process_ButtonsPressedThenExchangeRequested()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t numberOfButtons = 8;
    
    // First response with buttons pressed
    ScanResponse firstResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0010 // Button 0 pressed
    };
    bus.QueueResponse(firstResponse);
    
    // Second response when Exchange is called
    ScanResponse secondResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0010 // Button 0 still pressed
    };
    bus.QueueResponse(secondResponse);
    
    PushButtonModule module(bus, address, numberOfButtons);
    
    // Act - First process sets m_hasPressedButtons to true
    auto firstProcessResponse = module.Process();
    TEST_ASSERT_TRUE(firstProcessResponse.Success);
    
    // Now clear the calls to check the second process
    bus.ClearCalls();
    
    // Act - Second process should use Exchange instead of Poll
    auto secondProcessResponse = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(secondProcessResponse.Success);
    TEST_ASSERT_TRUE(secondProcessResponse.RaisePriority);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_TRUE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
    TEST_ASSERT_EQUAL(0x06, bus.LastExchangeData); // Command 6
}

void PushButtonModule_Process_FailedPoll()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t numberOfButtons = 8;
    
    // Setup mock response for failed poll
    ScanResponse failedResponse =
    {
        .Success = false
    };
    bus.QueueResponse(failedResponse);
    
    PushButtonModule module(bus, address, numberOfButtons);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_FALSE(response.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_FALSE(bus.LastForceDataExchange);
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(PushButtonModule_CreateFromInitialData_InitializesCorrectly);
    RUN_TEST(PushButtonModule_Process_SuccessfulPoll_NoButtonsPressed);
    RUN_TEST(PushButtonModule_Process_SuccessfulPoll_ButtonsPressed);
    RUN_TEST(PushButtonModule_Process_ButtonsReleased);
    RUN_TEST(PushButtonModule_Process_ButtonsPressedThenExchangeRequested);
    RUN_TEST(PushButtonModule_Process_FailedPoll);
    
    return UNITY_END();
}
