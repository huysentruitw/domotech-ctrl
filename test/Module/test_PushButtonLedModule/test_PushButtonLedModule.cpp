#include <unity.h>
#include <memory>

#include <Modules/PushButtonLedModule.h>
#include "../../Mocks/MockBus.h"

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void PushButtonLedModule_CreateFromInitialData_InitializesCorrectly()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x10;
    const uint16_t initialData = 0x4000; // 4 buttons

    // Act
    auto module = PushButtonLedModule::CreateFromInitialData(bus, address, initialData);

    // Assert
    auto outputs = module->GetOutputPins();
    auto inputs  = module->GetInputPins();

    TEST_ASSERT_EQUAL(4, outputs.size());
    TEST_ASSERT_EQUAL(4, inputs.size());

    // All button pins start false
    for (auto& weak : outputs)
    {
        auto pin = weak.lock();
        TEST_ASSERT_NOT_NULL(pin.get());
        TEST_ASSERT_EQUAL(DigitalValue(false), pin->GetStateAs<DigitalValue>());
    }
}

void PushButtonLedModule_Process_NoButtonsPressed()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 4;

    ScanResponse poll =
    {
        .Success = true,
        .RespondedWithTypeAndData = false
    };
    bus.QueueResponse(poll);

    PushButtonLedModule module(bus, address, numButtons);

    // Act
    auto r = module.Process();

    // Assert
    TEST_ASSERT_TRUE(r.Success);
    TEST_ASSERT_FALSE(r.BoostPriority);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_FALSE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(address, bus.LastExchangeAddress);
}

void PushButtonLedModule_Process_ButtonsPressed()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 4;

    // ButtonMasks4 = {0x08, 0x04, 0x02, 0x01}
    ScanResponse poll =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0008 // button 0 pressed
    };
    bus.QueueResponse(poll);

    PushButtonLedModule module(bus, address, numButtons);

    // Act
    auto r = module.Process();

    // Assert
    TEST_ASSERT_TRUE(r.Success);
    TEST_ASSERT_TRUE(r.BoostPriority);

    auto pins = module.GetOutputPins();
    TEST_ASSERT_EQUAL(DigitalValue(true), pins[0].lock()->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), pins[1].lock()->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), pins[2].lock()->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), pins[3].lock()->GetStateAs<DigitalValue>());
}

void PushButtonLedModule_Process_ButtonsReleased()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 4;

    ScanResponse first =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0008 // button 0 pressed
    };
    ScanResponse second{
        .Success = true,
        .RespondedWithTypeAndData = false
    };

    bus.QueueResponse(first);
    bus.QueueResponse(second);

    PushButtonLedModule module(bus, address, numButtons);

    // First: pressed
    auto r1 = module.Process();
    TEST_ASSERT_TRUE(r1.Success);
    TEST_ASSERT_TRUE(r1.BoostPriority);

    bus.ClearCalls();

    // Second: released
    auto r2 = module.Process();
    TEST_ASSERT_TRUE(r2.Success);
    TEST_ASSERT_FALSE(r2.BoostPriority);

    auto pins = module.GetOutputPins();
    for (auto& weak : pins)
        TEST_ASSERT_EQUAL(DigitalValue(false), weak.lock()->GetStateAs<DigitalValue>());
}

void PushButtonLedModule_Process_ButtonsPressedThenForcedExchange()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 4;

    ScanResponse first =
    {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0008
    };
    ScanResponse second{
        .Success = true,
        .RespondedWithTypeAndData = true,
        .Data = 0x0008
    };

    bus.QueueResponse(first);
    bus.QueueResponse(second);

    PushButtonLedModule module(bus, address, numButtons);

    auto r1 = module.Process();
    TEST_ASSERT_TRUE(r1.Success);

    bus.ClearCalls();

    auto r2 = module.Process();

    TEST_ASSERT_TRUE(r2.Success);
    TEST_ASSERT_TRUE(r2.BoostPriority);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_TRUE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(0x06, bus.LastExchangeData);
}

void PushButtonLedModule_Process_FailedPoll()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 4;

    ScanResponse fail =
    {
        .Success = false
    };
    bus.QueueResponse(fail);

    PushButtonLedModule module(bus, address, numButtons);

    // Act
    auto r = module.Process();

    // Assert
    TEST_ASSERT_FALSE(r.Success);
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
}

void PushButtonLedModule_UpdateLed_TurnOn()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 4;

    PushButtonLedModule module(bus, address, numButtons);

    bus.ClearCalls();

    // Act — simulate LED input pin change
    auto ledPin = module.GetInputPins()[0].lock();
    ledPin->SetState(DigitalValue(true));

    // Assert
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_TRUE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(0x81, bus.LastExchangeData); // CMD1 | (0x80 >> 0)
}

void PushButtonLedModule_UpdateLed_TurnOff()
{
    // Arrange
    MockBus bus;
    const uint8_t address = 0x20;
    const uint8_t numButtons = 4;

    PushButtonLedModule module(bus, address, numButtons);

    auto ledPin = module.GetInputPins()[0].lock();
    ledPin->SetState(DigitalValue(true));
    bus.ClearCalls();
 
    // Act
    ledPin->SetState(DigitalValue(false));

    // Assert
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_TRUE(bus.LastForceDataExchange);
    TEST_ASSERT_EQUAL(0x82, bus.LastExchangeData); // CMD2 | (0x80 >> 0)
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(PushButtonLedModule_CreateFromInitialData_InitializesCorrectly);
    RUN_TEST(PushButtonLedModule_Process_NoButtonsPressed);
    RUN_TEST(PushButtonLedModule_Process_ButtonsPressed);
    RUN_TEST(PushButtonLedModule_Process_ButtonsReleased);
    RUN_TEST(PushButtonLedModule_Process_ButtonsPressedThenForcedExchange);
    RUN_TEST(PushButtonLedModule_Process_FailedPoll);
    RUN_TEST(PushButtonLedModule_UpdateLed_TurnOn);
    RUN_TEST(PushButtonLedModule_UpdateLed_TurnOff);

    return UNITY_END();
}
