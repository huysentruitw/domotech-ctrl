#include <unity.h>
#include <memory>

#include <Modules/PushButtonTemperatureModule.h>
#include "../../Mocks/MockBus.h"

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void PushButtonTemperatureModule_CreateFromInitialData_InitializesCorrectly()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t initialData = 0x6000; // 6 buttons

    // Act
    auto module = PushButtonTemperatureModule::CreateFromInitialData(bus, address, initialData);

    // Assert
    auto pins = module->GetOutputPins();
    TEST_ASSERT_EQUAL(7, pins.size()); // 6 buttons + 1 temperature pin

    // First 6 pins = buttons, last = temperature
    for (size_t i = 0; i < 6; i++)
    {
        auto pin = pins[i].lock();
        TEST_ASSERT_NOT_NULL(pin.get());
        TEST_ASSERT_EQUAL(DigitalValue(false), pin->GetStateAs<DigitalValue>());
    }

    auto tempPin = pins.back().lock();
    TEST_ASSERT_NOT_NULL(tempPin.get());
    TEST_ASSERT_EQUAL((float)TemperatureValue(0), (float)tempPin->GetStateAs<TemperatureValue>());
}

void PushButtonTemperatureModule_Process_SuccessfulPoll_NoButtonsPressed()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 6;

    ScanResponse pollResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = false,
    };
    bus.QueueResponse(pollResponse);

    PushButtonTemperatureModule module(bus, address, numButtons);

    // Act
    auto response = module.Process();

    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_FALSE(response.BoostPriority);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_TRUE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
}

void PushButtonTemperatureModule_Process_SuccessfulPoll_ButtonsPressed()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 6;

    // ButtonMasks6[0] = 0x10
    ScanResponse pollResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0010
    };
    bus.QueueResponse(pollResponse);

    PushButtonTemperatureModule module(bus, address, numButtons);

    // Act
    auto response = module.Process();

    // Assert
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_TRUE(response.BoostPriority);

    auto pins = module.GetOutputPins();
    TEST_ASSERT_EQUAL(DigitalValue(true), pins[0].lock()->GetStateAs<DigitalValue>());
    for (size_t i = 1; i < numButtons; i++)
        TEST_ASSERT_EQUAL(DigitalValue(false), pins[i].lock()->GetStateAs<DigitalValue>());
}

void PushButtonTemperatureModule_Process_ButtonsReleased()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 6;

    ScanResponse buttonsPressedResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0010,
    };
    ScanResponse temperatureResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0015,
    };
    ScanResponse buttonsReleasedResponse =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0,
    };

    bus.QueueResponse(buttonsPressedResponse);
    bus.QueueResponse(temperatureResponse);
    bus.QueueResponse(buttonsReleasedResponse);

    PushButtonTemperatureModule module(bus, address, numButtons);

    // First process: button pressed
    auto r1 = module.Process();
    TEST_ASSERT_TRUE(r1.Success);
    TEST_ASSERT_TRUE(r1.BoostPriority);

    bus.ClearCalls();

    // Second process: released
    auto r2 = module.Process();
    TEST_ASSERT_TRUE(r2.Success);
    TEST_ASSERT_FALSE(r2.BoostPriority);

    auto pins = module.GetOutputPins();
    for (size_t i = 0; i < numButtons; i++)
        TEST_ASSERT_EQUAL(DigitalValue(false), pins[i].lock()->GetStateAs<DigitalValue>());
}

void PushButtonTemperatureModule_Process_FailedPoll()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 6;

    ScanResponse fail =
    {
        .Success = false
    };
    bus.QueueResponse(fail);

    PushButtonTemperatureModule module(bus, address, numButtons);

    // Act
    auto response = module.Process();

    // Assert
    TEST_ASSERT_FALSE(response.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
}

void PushButtonTemperatureModule_Process_TemperatureRequested()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 6;

    // First: button state response (no buttons pressed)
    ScanResponse poll =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0000
    };

    // Second: temperature response
    ScanResponse temp =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x000A // raw = 10 → 10 * 0.5 = 5.0°C
    };

    bus.QueueResponse(poll);
    bus.QueueResponse(temp);

    PushButtonTemperatureModule module(bus, address, numButtons);

    // Act
    auto r = module.Process();

    // Assert
    TEST_ASSERT_TRUE(r.Success);
    TEST_ASSERT_FALSE(r.BoostPriority);

    auto pins = module.GetOutputPins();
    auto tempPin = pins.back().lock();
    TEST_ASSERT_EQUAL((float)TemperatureValue(5.0f), (float)tempPin->GetStateAs<TemperatureValue>());
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(PushButtonTemperatureModule_CreateFromInitialData_InitializesCorrectly);
    RUN_TEST(PushButtonTemperatureModule_Process_SuccessfulPoll_NoButtonsPressed);
    RUN_TEST(PushButtonTemperatureModule_Process_SuccessfulPoll_ButtonsPressed);
    RUN_TEST(PushButtonTemperatureModule_Process_ButtonsReleased);
    RUN_TEST(PushButtonTemperatureModule_Process_FailedPoll);
    RUN_TEST(PushButtonTemperatureModule_Process_TemperatureRequested);
    
    return UNITY_END();
}
