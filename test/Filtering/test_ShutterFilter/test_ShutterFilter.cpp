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
    for (auto i = 0; i < 10; i++) static_cast<Filter&>(filter).Every100Ms();
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
    for (auto i = 0; i < 10; i++) static_cast<Filter&>(filter).Every100Ms();
    closePin->SetState(DigitalValue(false));

    // Assert: Close output should now be false
    TEST_ASSERT_EQUAL(DigitalValue(false), closeOutput->GetStateAs<DigitalValue>());
}

void ShutterFilter_OpenWhileClosing_NoFeedback_ReleasesCloseOutput()
{
    // Arrange
    ShutterFilter filter;
    auto openOutput = filter.GetOpenOutputPin().lock();
    auto closeOutput = filter.GetCloseOutputPin().lock();
    filter.Close();

    // Act
    filter.Open();

    // Assert: never both directions at once
    TEST_ASSERT_EQUAL(DigitalValue(true), openOutput->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), closeOutput->GetStateAs<DigitalValue>());
}

void ShutterFilter_CloseWhileOpening_NoFeedback_ReleasesOpenOutput()
{
    // Arrange
    ShutterFilter filter;
    auto openOutput = filter.GetOpenOutputPin().lock();
    auto closeOutput = filter.GetCloseOutputPin().lock();
    filter.Open();

    // Act
    filter.Close();

    // Assert: never both directions at once
    TEST_ASSERT_EQUAL(DigitalValue(false), openOutput->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(true), closeOutput->GetStateAs<DigitalValue>());
}

void ShutterFilter_ReversalViaInputPins_NoFeedback_NeverDrivesBothOutputs()
{
    // Arrange
    ShutterFilter filter;
    auto openPin = filter.GetOpenInputPin().lock();
    auto closePin = filter.GetCloseInputPin().lock();
    auto openOutput = filter.GetOpenOutputPin().lock();
    auto closeOutput = filter.GetCloseOutputPin().lock();

    // Act: short press close, then short press open
    closePin->SetState(DigitalValue(true));
    closePin->SetState(DigitalValue(false));
    openPin->SetState(DigitalValue(true));

    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), openOutput->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), closeOutput->GetStateAs<DigitalValue>());
}

void ShutterFilter_OpenWhileClosing_WithFeedback_WaitsForCloseFeedbackToDrop()
{
    // Arrange
    ShutterFilter filter;
    auto closeFeedbackPin = filter.GetCloseFeedbackInputPin().lock();
    auto openOutput = filter.GetOpenOutputPin().lock();
    auto closeOutput = filter.GetCloseOutputPin().lock();
    filter.Close();
    closeFeedbackPin->SetState(DigitalValue(true)); // Close relay reports active

    // Act
    filter.Open();

    // Assert: close released, open not yet energized
    TEST_ASSERT_EQUAL(DigitalValue(false), closeOutput->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), openOutput->GetStateAs<DigitalValue>());

    // Act: close relay reports released
    closeFeedbackPin->SetState(DigitalValue(false));

    // Assert: open now energized
    TEST_ASSERT_EQUAL(DigitalValue(true), openOutput->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), closeOutput->GetStateAs<DigitalValue>());
}

void ShutterFilter_CloseWhileOpening_WithFeedback_WaitsForOpenFeedbackToDrop()
{
    // Arrange
    ShutterFilter filter;
    auto openFeedbackPin = filter.GetOpenFeedbackInputPin().lock();
    auto openOutput = filter.GetOpenOutputPin().lock();
    auto closeOutput = filter.GetCloseOutputPin().lock();
    filter.Open();
    openFeedbackPin->SetState(DigitalValue(true)); // Open relay reports active

    // Act
    filter.Close();

    // Assert: open released, close not yet energized
    TEST_ASSERT_EQUAL(DigitalValue(false), openOutput->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), closeOutput->GetStateAs<DigitalValue>());

    // Act: open relay reports released
    openFeedbackPin->SetState(DigitalValue(false));

    // Assert: close now energized
    TEST_ASSERT_EQUAL(DigitalValue(false), openOutput->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(true), closeOutput->GetStateAs<DigitalValue>());
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();

    RUN_TEST(ShutterFilter_Constructor_CreatesPins);
    RUN_TEST(ShutterFilter_OpenInput_LongPress_ChangesOpenOutputAndStops);
    RUN_TEST(ShutterFilter_OpenInput_ShortPress_ChangesOpenOutputAndRemains);
    RUN_TEST(ShutterFilter_CloseInput_LongPress_ChangesCloseOutputAndStops);
    RUN_TEST(ShutterFilter_CloseInput_ShortPress_ChangesCloseOutputAndRemains);
    RUN_TEST(ShutterFilter_OpenWhileClosing_NoFeedback_ReleasesCloseOutput);
    RUN_TEST(ShutterFilter_CloseWhileOpening_NoFeedback_ReleasesOpenOutput);
    RUN_TEST(ShutterFilter_ReversalViaInputPins_NoFeedback_NeverDrivesBothOutputs);
    RUN_TEST(ShutterFilter_OpenWhileClosing_WithFeedback_WaitsForCloseFeedbackToDrop);
    RUN_TEST(ShutterFilter_CloseWhileOpening_WithFeedback_WaitsForOpenFeedbackToDrop);

    return UNITY_END();
}
