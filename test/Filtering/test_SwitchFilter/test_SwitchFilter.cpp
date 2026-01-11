#include <unity.h>
#include <memory>

#include <Filters/SwitchFilter.h>
#include <PinFactory.h>

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void SwitchFilter_Constructor_CreatesPins()
{
    // Arrange
    SwitchFilter filter;

    // Act
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    // Assert
    TEST_ASSERT_EQUAL(2, inputPins.size());
    TEST_ASSERT_EQUAL(2, outputPins.size());

    auto togglePin = inputPins[0].lock();
    auto feedbackInputPin = inputPins[1].lock();
    auto controlPin = outputPins[0].lock();
    auto feedbackOutputPin = outputPins[1].lock();

    TEST_ASSERT_NOT_NULL(togglePin.get());
    TEST_ASSERT_NOT_NULL(feedbackInputPin.get());
    TEST_ASSERT_NOT_NULL(controlPin.get());
    TEST_ASSERT_NOT_NULL(feedbackOutputPin.get());

    TEST_ASSERT_EQUAL(DigitalValue(false), togglePin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), feedbackInputPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), controlPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), feedbackOutputPin->GetStateAs<DigitalValue>());
}

void SwitchFilter_ToggleInput_ChangesControlOutput()
{
    // Arrange
    SwitchFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto togglePin = inputPins[0].lock();
    auto controlPin = outputPins[0].lock();
    auto feedbackOutputPin = outputPins[1].lock();

    // Act: Set toggle input to true
    togglePin->SetState(DigitalValue(true));

    // Assert: Control & feedback output should now be true
    TEST_ASSERT_EQUAL(DigitalValue(true), controlPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(true), feedbackOutputPin->GetStateAs<DigitalValue>());

    // Act: Set toggle input to false
    togglePin->SetState(DigitalValue(false));

    // Assert: Control & feedback output should still be true
    TEST_ASSERT_EQUAL(DigitalValue(true), controlPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(true), feedbackOutputPin->GetStateAs<DigitalValue>());

    // Act: Set toggle input to true again
    togglePin->SetState(DigitalValue(true));

    // Assert: Control & feedback output should now be false
    TEST_ASSERT_EQUAL(DigitalValue(false), controlPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), feedbackOutputPin->GetStateAs<DigitalValue>());

    // Act: Set toggle input to false again
    togglePin->SetState(DigitalValue(false));

    // Assert: Control & feedback output should still be false
    TEST_ASSERT_EQUAL(DigitalValue(false), controlPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), feedbackOutputPin->GetStateAs<DigitalValue>());
}

void SwitchFilter_FeedbackInput_ForwardsToFeedbackOutput()
{
    // Arrange
    SwitchFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto togglePin = inputPins[0].lock();
    auto feedbackInputPin = inputPins[1].lock();
    auto feedbackOutputPin = outputPins[1].lock();

    auto testFeedbackPin = PinFactory::CreateOutputPin<DigitalValue>();
    Pin::Connect(feedbackInputPin, testFeedbackPin);

    // Assert: Initial state of feedback output is false
    TEST_ASSERT_EQUAL(DigitalValue(false), feedbackOutputPin->GetStateAs<DigitalValue>());

    // Act & Assert: Toggle filter does not change feedback output because feedback input is connected
    togglePin->SetState(DigitalValue(true));
    TEST_ASSERT_EQUAL(DigitalValue(false), feedbackOutputPin->GetStateAs<DigitalValue>());

    // Act & Assert: On feedback true, feedback input is forwarded to feedback output
    testFeedbackPin->SetState(DigitalValue(true));
    TEST_ASSERT_EQUAL(DigitalValue(true), feedbackOutputPin->GetStateAs<DigitalValue>());

    // Act & Assert: On feedback false, feedback input is forwarded to feedback output
    testFeedbackPin->SetState(DigitalValue(false));
    TEST_ASSERT_EQUAL(DigitalValue(false), feedbackOutputPin->GetStateAs<DigitalValue>());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(SwitchFilter_Constructor_CreatesPins);
    RUN_TEST(SwitchFilter_ToggleInput_ChangesControlOutput);
    RUN_TEST(SwitchFilter_FeedbackInput_ForwardsToFeedbackOutput);

    return UNITY_END();
}
