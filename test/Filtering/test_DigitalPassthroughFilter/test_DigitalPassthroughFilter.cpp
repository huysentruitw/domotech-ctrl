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

int main()
{
    UNITY_BEGIN();

    RUN_TEST(DigitalPassthroughFilter_Constructor_CreatesPins);
    RUN_TEST(DigitalPassthroughFilter_InputChangesOutput);

    return UNITY_END();
}
