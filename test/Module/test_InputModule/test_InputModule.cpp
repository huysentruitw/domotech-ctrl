#include <unity.h>
#include <memory>

#include <Modules/InputModule.h>
#include "../../Mocks/MockBus.h"

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void InputModule_CreateFromInitialData_InitializesCorrectly()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint16_t initialData = 0x3000; // 3 inputs

    // Act
    auto module = InputModule::CreateFromInitialData(bus, address, initialData);

    // Assert
    auto outputPins = module->GetOutputPins();
    TEST_ASSERT_EQUAL(3, outputPins.size());

    for (const auto& weakPin : outputPins)
    {
        auto pin = weakPin.lock();
        TEST_ASSERT_NOT_NULL(pin.get());
        TEST_ASSERT_EQUAL(DigitalValue(false), pin->GetStateAs<DigitalValue>());
    }
}

void InputModule_Process_FirstPoll_ForcesExchange()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numberOfInputs = 4;

    ScanResponse response =
    {
        .Success = true,
        .RespondedWithTypeAndData = false
    };
    bus.QueueResponse(response);

    InputModule module(bus, address, numberOfInputs);

    // Act
    auto result = module.Process();

    // Assert
    TEST_ASSERT_TRUE(result.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_TRUE(bus.LastForceDataExchange); // first call forces exchange
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
}

void InputModule_Process_SuccessfulPoll_NoInputsActive()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numberOfInputs = 8;

    ScanResponse response =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0000
    };
    bus.QueueResponse(response);

    InputModule module(bus, address, numberOfInputs);

    // Act
    auto result = module.Process();

    // Assert
    TEST_ASSERT_TRUE(result.Success);

    auto pins = module.GetOutputPins();
    for (const auto& weakPin : pins)
        TEST_ASSERT_EQUAL(DigitalValue(false), weakPin.lock()->GetStateAs<DigitalValue>());
}

void InputModule_Process_SuccessfulPoll_InputsActive()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numberOfInputs = 8;

    // InputMasks8[0] = 0x10 → bit 4
    ScanResponse response =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0010
    };
    bus.QueueResponse(response);

    InputModule module(bus, address, numberOfInputs);

    // Act
    auto result = module.Process();

    // Assert
    TEST_ASSERT_TRUE(result.Success);

    auto pins = module.GetOutputPins();
    TEST_ASSERT_EQUAL(DigitalValue(true), pins[0].lock()->GetStateAs<DigitalValue>());
    for (size_t i = 1; i < pins.size(); i++)
        TEST_ASSERT_EQUAL(DigitalValue(false), pins[i].lock()->GetStateAs<DigitalValue>());
}

void InputModule_Process_SecondPoll_DoesNotForceExchange()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numberOfInputs = 8;

    // First response: data
    ScanResponse first =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0010
    };
    bus.QueueResponse(first);

    // Second response: no data
    ScanResponse second =
    {
        .Success = true,
        .RespondedWithTypeAndData = false
    };
    bus.QueueResponse(second);

    InputModule module(bus, address, numberOfInputs);

    // First call forces exchange
    auto firstResult = module.Process();
    TEST_ASSERT_TRUE(firstResult.Success);

    bus.ClearCalls();

    // Act – second call should NOT force exchange
    auto secondResult = module.Process();

    // Assert
    TEST_ASSERT_TRUE(secondResult.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_FALSE(bus.LastForceDataExchange);
}

void InputModule_Process_FailedPoll()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numberOfInputs = 8;

    ScanResponse failed =
    {
        .Success = false
    };
    bus.QueueResponse(failed);

    InputModule module(bus, address, numberOfInputs);

    // Act
    auto result = module.Process();

    // Assert
    TEST_ASSERT_FALSE(result.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_TRUE(bus.LastForceDataExchange); // still forced on first call
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(InputModule_CreateFromInitialData_InitializesCorrectly);
    RUN_TEST(InputModule_Process_FirstPoll_ForcesExchange);
    RUN_TEST(InputModule_Process_SuccessfulPoll_NoInputsActive);
    RUN_TEST(InputModule_Process_SuccessfulPoll_InputsActive);
    RUN_TEST(InputModule_Process_SecondPoll_DoesNotForceExchange);
    RUN_TEST(InputModule_Process_FailedPoll);

    return UNITY_END();
}
