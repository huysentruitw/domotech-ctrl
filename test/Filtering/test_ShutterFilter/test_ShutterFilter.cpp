#include <unity.h>
#include <memory>

#include <Filters/ShutterFilter.h>

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void ShutterFilter_Constructor_CreatesPins()
{
    // Arrange
    ShutterFilter filter;

    // Act
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    // Assert
    TEST_ASSERT_EQUAL(5, inputPins.size());
    TEST_ASSERT_EQUAL(2, outputPins.size());

    auto openPin = inputPins[0].lock();
    auto closePin = inputPins[1].lock();
    auto stopPin = inputPins[2].lock();
    auto openFeedbackPin = inputPins[3].lock();
    auto closeFeedbackPin = inputPins[4].lock();

    TEST_ASSERT_NOT_NULL(openPin.get());
    TEST_ASSERT_NOT_NULL(closePin.get());
    TEST_ASSERT_NOT_NULL(stopPin.get());
    TEST_ASSERT_NOT_NULL(openFeedbackPin.get());
    TEST_ASSERT_NOT_NULL(closeFeedbackPin.get());

    TEST_ASSERT_EQUAL(DigitalValue(false), openPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), closePin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), stopPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), openFeedbackPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), closeFeedbackPin->GetStateAs<DigitalValue>());

    auto openOutput = outputPins[0].lock();
    auto closeOutput = outputPins[1].lock();

    TEST_ASSERT_NOT_NULL(openOutput.get());
    TEST_ASSERT_NOT_NULL(closeOutput.get());

    TEST_ASSERT_EQUAL(DigitalValue(false), openOutput->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), closeOutput->GetStateAs<DigitalValue>());
}

void ShutterFilter_OpenInput_ShortPress_ChangesOpenOutputAndRemains()
{
    // Arrange
    ShutterFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto openPin = inputPins[0].lock();
    auto openOutput = outputPins[0].lock();

    // Act: Set open input to true
    openPin->SetState(DigitalValue(true));

    // Assert: Open output should now be true
    TEST_ASSERT_EQUAL(DigitalValue(true), openOutput->GetStateAs<DigitalValue>());

    // Act: Set open input to false
    openPin->SetState(DigitalValue(false));

    // Assert: Open output should remain true
    TEST_ASSERT_EQUAL(DigitalValue(true), openOutput->GetStateAs<DigitalValue>());
}

void ShutterFilter_OpenInput_LongPress_ChangesOpenOutputAndStops()
{
    // Arrange
    ShutterFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto openPin = inputPins[0].lock();
    auto openOutput = outputPins[0].lock();

    // Act: Set open input to true
    openPin->SetState(DigitalValue(true));

    // Assert: Open output should now be true
    TEST_ASSERT_EQUAL(DigitalValue(true), openOutput->GetStateAs<DigitalValue>());

    // Act: Set open input to false (simulate long press)
    filter.MoveSignalStartMs(-1100);
    openPin->SetState(DigitalValue(false));

    // Assert: Open output should now be false
    TEST_ASSERT_EQUAL(DigitalValue(false), openOutput->GetStateAs<DigitalValue>());
}

void ShutterFilter_CloseInput_ShortPress_ChangesCloseOutputAndRemains()
{
    // Arrange
    ShutterFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto closePin = inputPins[1].lock();
    auto closeOutput = outputPins[1].lock();

    // Act: Set close input to true
    closePin->SetState(DigitalValue(true));

    // Assert: Close output should now be true
    TEST_ASSERT_EQUAL(DigitalValue(true), closeOutput->GetStateAs<DigitalValue>());

    // Act: Set close input to false
    closePin->SetState(DigitalValue(false));

    // Assert: Close output should remain true
    TEST_ASSERT_EQUAL(DigitalValue(true), closeOutput->GetStateAs<DigitalValue>());
}

void ShutterFilter_CloseInput_LongPress_ChangesCloseOutputAndStops()
{
    // Arrange
    ShutterFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto closePin = inputPins[1].lock();
    auto closeOutput = outputPins[1].lock();

    // Act: Set close input to true
    closePin->SetState(DigitalValue(true));

    // Assert: Close output should now be true
    TEST_ASSERT_EQUAL(DigitalValue(true), closeOutput->GetStateAs<DigitalValue>());

    // Act: Set close input to false (simulate long press)
    filter.MoveSignalStartMs(-1100);
    closePin->SetState(DigitalValue(false));

    // Assert: Close output should now be false
    TEST_ASSERT_EQUAL(DigitalValue(false), closeOutput->GetStateAs<DigitalValue>());
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();

    RUN_TEST(ShutterFilter_Constructor_CreatesPins);
    RUN_TEST(ShutterFilter_OpenInput_LongPress_ChangesOpenOutputAndStops);
    RUN_TEST(ShutterFilter_OpenInput_ShortPress_ChangesOpenOutputAndRemains);
    RUN_TEST(ShutterFilter_CloseInput_LongPress_ChangesCloseOutputAndStops);
    RUN_TEST(ShutterFilter_CloseInput_ShortPress_ChangesCloseOutputAndRemains);

    // TODO: Add feedback input tests

    return UNITY_END();
}
