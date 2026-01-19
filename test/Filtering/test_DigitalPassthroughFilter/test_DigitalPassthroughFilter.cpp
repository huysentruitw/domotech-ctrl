#include <unity.h>
#include <memory>

#include <Filters/DigitalPassthroughFilter.h>

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void DigitalPassthroughFilter_Constructor_CreatesPins()
{
    // Arrange
    DigitalPassthroughFilter filter;

    // Act
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    // Assert
    TEST_ASSERT_EQUAL(1, inputPins.size());
    TEST_ASSERT_EQUAL(1, outputPins.size());

    auto inputPin = inputPins[0].lock();
    auto outputPin = outputPins[0].lock();

    TEST_ASSERT_NOT_NULL(inputPin.get());
    TEST_ASSERT_NOT_NULL(outputPin.get());

    // Default states
    TEST_ASSERT_EQUAL(DigitalValue(false), inputPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), outputPin->GetStateAs<DigitalValue>());
}

void DigitalPassthroughFilter_InputChangesOutput()
{
    // Arrange
    DigitalPassthroughFilter filter;
    auto inputPin = filter.GetInputPins()[0].lock();
    auto outputPin = filter.GetOutputPins()[0].lock();

    // Act: Set input to true
    inputPin->SetState(DigitalValue(true));

    // Assert: Output should now be true
    TEST_ASSERT_EQUAL(DigitalValue(true), outputPin->GetStateAs<DigitalValue>());

    // Act: Set input to false
    inputPin->SetState(DigitalValue(false));

    // Assert: Output should now be false
    TEST_ASSERT_EQUAL(DigitalValue(false), outputPin->GetStateAs<DigitalValue>());
}

void DigitalPassthroughFilter_Callback_IsCalled_OnChange()
{
    // Arrange
    DigitalPassthroughFilter filter;
    auto inputPin = filter.GetInputPins()[0].lock();

    bool callbackCalled = false;
    DigitalValue callbackValue(false);

    filter.SetStateCallback(
        [&](const DigitalPassthroughFilter&, DigitalValue value)
        {
            callbackCalled = true;
            callbackValue = value;
        });

    // Act: Change input to true
    inputPin->SetState(DigitalValue(true));

    // Assert
    TEST_ASSERT_TRUE(callbackCalled);
    TEST_ASSERT_EQUAL(DigitalValue(true), callbackValue);
}

void DigitalPassthroughFilter_Callback_NotCalled_WhenStateUnchanged()
{
    // Arrange
    DigitalPassthroughFilter filter;
    auto inputPin = filter.GetInputPins()[0].lock();

    bool callbackCalled = false;

    filter.SetStateCallback(
        [&](const DigitalPassthroughFilter&, DigitalValue)
        {
            callbackCalled = true;
        });

    // Act: Set input to false (already false)
    inputPin->SetState(DigitalValue(false));

    // Assert
    TEST_ASSERT_FALSE(callbackCalled);
}

void DigitalPassthroughFilter_SetStateCallback_OnlyOnce()
{
    // Arrange
    DigitalPassthroughFilter filter;

    auto first = [&](const DigitalPassthroughFilter&, DigitalValue) {};
    auto second = [&](const DigitalPassthroughFilter&, DigitalValue) {};

    // Act
    bool firstResult = filter.SetStateCallback(first);
    bool secondResult = filter.SetStateCallback(second);

    // Assert
    TEST_ASSERT_TRUE(firstResult);
    TEST_ASSERT_FALSE(secondResult);
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(DigitalPassthroughFilter_Constructor_CreatesPins);
    RUN_TEST(DigitalPassthroughFilter_InputChangesOutput);
    RUN_TEST(DigitalPassthroughFilter_Callback_IsCalled_OnChange);
    RUN_TEST(DigitalPassthroughFilter_Callback_NotCalled_WhenStateUnchanged);
    RUN_TEST(DigitalPassthroughFilter_SetStateCallback_OnlyOnce);

    return UNITY_END();
}
