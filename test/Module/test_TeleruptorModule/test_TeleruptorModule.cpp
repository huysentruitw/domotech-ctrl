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

void TeleruptorModule_CreateFromInitialData_InitializesCorrectly()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t initialData = 0x0004; // 4 teleruptors
    
    // Act
    auto module = TeleruptorModule::CreateFromInitialData(bus, address, initialData);
    
    // Assert
    auto inputPins = module->GetInputPins();
    auto outputPins = module->GetOutputPins();
    
    TEST_ASSERT_EQUAL(4, inputPins.size());
    TEST_ASSERT_EQUAL(4, outputPins.size());
    
    // Verify each input pin is initially set to false
    for (const auto& weakPin : inputPins)
    {
        auto pin = weakPin.lock();
        TEST_ASSERT_NOT_NULL(pin.get());
        TEST_ASSERT_EQUAL(DigitalValue(false), pin->GetStateAs<DigitalValue>());
    }
    
    // Verify each output pin is initially set to false
    for (const auto& weakPin : outputPins)
    {
        auto pin = weakPin.lock();
        TEST_ASSERT_NOT_NULL(pin.get());
        TEST_ASSERT_EQUAL(DigitalValue(false), pin->GetStateAs<DigitalValue>());
    }
}

void TeleruptorModule_Process_InitialExchange_RequestFeedbackState()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t numberOfTeleruptors = 4;
    
    // Setup mock response for poll
    ScanResponse pollResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = false,
    };
    bus.QueueResponse(pollResponse);
    
    TeleruptorModule module(bus, address, numberOfTeleruptors);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_TRUE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
}

void TeleruptorModule_Process_SubsequentExchange_NoFeedbackStateChange()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t numberOfTeleruptors = 4;
    
    // Setup mock response for poll
    ScanResponse initialResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
    };
    ScanResponse subsequentResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = false,
    };
    bus.QueueResponse(initialResponse);
    bus.QueueResponse(subsequentResponse);

    TeleruptorModule module(bus, address, numberOfTeleruptors);
    
    // Act
    module.Process();
    bus.ClearCalls(); // Ignore initial exchange
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_FALSE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
}

void TeleruptorModule_Process_SubsequentExchange_WithFeedbackStateChange()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t numberOfTeleruptors = 4;
    
    // Setup mock response for poll
    ScanResponse initialResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
    };
    ScanResponse subsequentResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0003 // First two teleruptors are ON
    };
    bus.QueueResponse(initialResponse);
    bus.QueueResponse(subsequentResponse);

    TeleruptorModule module(bus, address, numberOfTeleruptors);
    
    // Act
    module.Process();
    bus.ClearCalls(); // Ignore initial exchange
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_FALSE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);

    // Verify feedback pins state
    auto outputPins = module.GetOutputPins();
    TEST_ASSERT_EQUAL(DigitalValue(true), outputPins[0].lock()->GetStateAs<DigitalValue>());  // First teleruptor ON
    TEST_ASSERT_EQUAL(DigitalValue(true), outputPins[1].lock()->GetStateAs<DigitalValue>());  // Second teleruptor ON
    TEST_ASSERT_EQUAL(DigitalValue(false), outputPins[2].lock()->GetStateAs<DigitalValue>()); // Third teleruptor OFF
    TEST_ASSERT_EQUAL(DigitalValue(false), outputPins[3].lock()->GetStateAs<DigitalValue>()); // Fourth teleruptor OFF
}

void TeleruptorModule_Process_FailedPoll()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t numberOfTeleruptors = 4;
    
    // Setup mock response for failed poll
    ScanResponse failedResponse =
    {
        .Success = false
    };
    bus.QueueResponse(failedResponse);
    
    TeleruptorModule module(bus, address, numberOfTeleruptors);
    
    // Act
    auto response = module.Process();
    
    // Assert
    TEST_ASSERT_FALSE(response.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_TRUE(bus.LastForceDataExchange);
}

void TeleruptorModule_UpdateTeleruptorState()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t numberOfTeleruptors = 4;
    
    // Setup mock response for Exchange
    ScanResponse exchangeResponse =
    {
        .Success = true
    };
    bus.QueueResponse(exchangeResponse);
    
    TeleruptorModule module(bus, address, numberOfTeleruptors);
    
    // Get input pin and set its state to trigger UpdateTeleruptorState
    auto inputPins = module.GetInputPins();
    inputPins[2].lock()->SetState(DigitalValue(true)); // Set third teleruptor to ON
    
    // Assert
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
    TEST_ASSERT_EQUAL(0x21, bus.LastExchangeData); // CMD1 (0x01) | (teleruptorIndex(2) << 4) = 0x21
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(TeleruptorModule_CreateFromInitialData_InitializesCorrectly);
    RUN_TEST(TeleruptorModule_Process_InitialExchange_RequestFeedbackState);
    RUN_TEST(TeleruptorModule_Process_SubsequentExchange_NoFeedbackStateChange);
    RUN_TEST(TeleruptorModule_Process_SubsequentExchange_WithFeedbackStateChange);
    RUN_TEST(TeleruptorModule_Process_FailedPoll);
    RUN_TEST(TeleruptorModule_UpdateTeleruptorState);
    
    return UNITY_END();
}
