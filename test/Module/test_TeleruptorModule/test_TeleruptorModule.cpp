#include <unity.h>
#include <memory>

#include <Modules/TeleruptorModule.h>
#include "../../Mocks/MockBus.h"

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void TeleruptorModule_Constructor_InitializesCorrectly()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t initialData = 0x0004; // 4 teleruptors
    
    // Act
    TeleruptorModule module(bus, address, initialData);
    
    // Assert
    auto inputPins = module.GetDigitalInputPins();
    auto outputPins = module.GetDigitalOutputPins();
    
    TEST_ASSERT_EQUAL(4, inputPins.size());
    TEST_ASSERT_EQUAL(4, outputPins.size());
    
    // Verify each input pin is initially set to false
    for (const auto& weakPin : inputPins) {
        auto pin = weakPin.lock();
        TEST_ASSERT_NOT_NULL(pin.get());
    }
    
    // Verify each output pin is initially set to false
    for (const auto& weakPin : outputPins) {
        auto pin = weakPin.lock();
        TEST_ASSERT_NOT_NULL(pin.get());
        TEST_ASSERT_FALSE(pin->GetState());
    }
}

void TeleruptorModule_Process_SuccessfulPoll_NoFeedbackChange()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t initialData = 0x0004; // 4 teleruptors
    
    // Setup mock response for poll
    ScanResponse pollResponse = {
        .Success = true,
        .RespondedWithTypeAndData = false
    };
    ScanResponse exchangeResponse = {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0000 // No feedback change
    };
    bus.QueueResponse(pollResponse);
    bus.QueueResponse(exchangeResponse);
    
    TeleruptorModule module(bus, address, initialData);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_TRUE(bus.PollCalled);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(address, bus.LastPolledAddress);
}

void TeleruptorModule_Process_WithFeedbackChange()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t initialData = 0x0004; // 4 teleruptors
    
    // Setup mock responses
    // First poll response indicates change in feedback
    ScanResponse pollResponse = {
        .Success = true,
        .RespondedWithTypeAndData = true
    };
    bus.QueueResponse(pollResponse);
    
    // Second response for feedback state request
    ScanResponse feedbackResponse = {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0003 // First two teleruptors are ON
    };
    bus.QueueResponse(feedbackResponse);
    
    TeleruptorModule module(bus, address, initialData);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_TRUE(bus.PollCalled);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
    TEST_ASSERT_EQUAL(0x06, bus.LastExchangeData); // Command 6 - Request feedback state
    
    // Verify feedback pins state
    auto outputPins = module.GetDigitalOutputPins();
    TEST_ASSERT_TRUE(outputPins[0].lock()->GetState());  // First teleruptor ON
    TEST_ASSERT_TRUE(outputPins[1].lock()->GetState());  // Second teleruptor ON
    TEST_ASSERT_FALSE(outputPins[2].lock()->GetState()); // Third teleruptor OFF
    TEST_ASSERT_FALSE(outputPins[3].lock()->GetState()); // Fourth teleruptor OFF
}

void TeleruptorModule_Process_FailedPoll()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t initialData = 0x0004; // 4 teleruptors
    
    // Setup mock response for failed poll
    ScanResponse failedResponse = {
        .Success = false
    };
    bus.QueueResponse(failedResponse);
    
    TeleruptorModule module(bus, address, initialData);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_FALSE(response.Success);
    TEST_ASSERT_TRUE(bus.PollCalled);
}

void TeleruptorModule_Process_FailedFeedbackExchange()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t initialData = 0x0004; // 4 teleruptors
    
    // Setup mock responses
    // First poll response indicates change in feedback
    ScanResponse pollResponse = {
        .Success = true,
        .RespondedWithTypeAndData = true
    };
    bus.QueueResponse(pollResponse);
    
    // Second response for feedback state request - fails
    ScanResponse failedFeedbackResponse = {
        .Success = false
    };
    bus.QueueResponse(failedFeedbackResponse);
    
    TeleruptorModule module(bus, address, initialData);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_FALSE(response.Success);
    TEST_ASSERT_TRUE(bus.PollCalled);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
}

void TeleruptorModule_UpdateTeleruptorState()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t initialData = 0x0004; // 4 teleruptors
    
    // Setup mock response for Exchange
    ScanResponse exchangeResponse = {
        .Success = true
    };
    bus.QueueResponse(exchangeResponse);
    
    TeleruptorModule module(bus, address, initialData);
    
    // Get input pin and set its state to trigger UpdateTeleruptorState
    auto inputPins = module.GetDigitalInputPins();
    inputPins[2].lock()->SetState(DigitalValue(true)); // Set third teleruptor to ON
    
    // Assert
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
    TEST_ASSERT_EQUAL(0x21, bus.LastExchangeData); // CMD1 (0x01) | (teleruptorIndex(2) << 4) = 0x21
}

void TeleruptorModule_ToString()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t initialData = 0x0004; // 4 teleruptors
    
    TeleruptorModule module(bus, address, initialData);
    
    // Act
    std::string result = module.ToString();
    
    // Assert
    TEST_ASSERT_EQUAL_STRING("TEL 32 4", result.c_str());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(TeleruptorModule_Constructor_InitializesCorrectly);
    RUN_TEST(TeleruptorModule_Process_SuccessfulPoll_NoFeedbackChange);
    RUN_TEST(TeleruptorModule_Process_WithFeedbackChange);
    RUN_TEST(TeleruptorModule_Process_FailedPoll);
    RUN_TEST(TeleruptorModule_Process_FailedFeedbackExchange);
    RUN_TEST(TeleruptorModule_UpdateTeleruptorState);
    RUN_TEST(TeleruptorModule_ToString);
    
    return UNITY_END();
}
