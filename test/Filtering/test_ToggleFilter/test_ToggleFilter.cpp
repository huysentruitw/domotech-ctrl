#include <unity.h>
#include <memory>

#include <ToggleFilter.h>

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void ToggleFilter_Constructor_CreatesPins()
{
    // Arrange
    ToggleFilter filter;

    // Act
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    // Assert
    TEST_ASSERT_EQUAL(1, inputPins.size());
    TEST_ASSERT_EQUAL(1, outputPins.size());

    auto togglePin = inputPins[0].lock();
    auto controlPin = outputPins[0].lock();

    TEST_ASSERT_NOT_NULL(togglePin.get());
    TEST_ASSERT_NOT_NULL(controlPin.get());

    TEST_ASSERT_EQUAL(DigitalValue(false), togglePin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), controlPin->GetStateAs<DigitalValue>());
}

void ToggleFilter_ToggleInput_ChangesControlOutput()
{
    // Arrange
    ToggleFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto togglePin = inputPins[0].lock();
    auto controlPin = outputPins[0].lock();

    // Act: Set toggle input to true
    togglePin->SetState(DigitalValue(true));

    // Assert: Control output should now be true
    TEST_ASSERT_EQUAL(DigitalValue(true), controlPin->GetStateAs<DigitalValue>());

    // Act: Set toggle input to false
    togglePin->SetState(DigitalValue(false));

    // Assert: Control output should still be true
    TEST_ASSERT_EQUAL(DigitalValue(true), controlPin->GetStateAs<DigitalValue>());

    // Act: Set toggle input to true again
    togglePin->SetState(DigitalValue(true));

    // Assert: Control output should now be false
    TEST_ASSERT_EQUAL(DigitalValue(false), controlPin->GetStateAs<DigitalValue>());

    // Act: Set toggle input to false again
    togglePin->SetState(DigitalValue(false));

    // Assert: Control output should still be false
    TEST_ASSERT_EQUAL(DigitalValue(false), controlPin->GetStateAs<DigitalValue>());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(ToggleFilter_Constructor_CreatesPins);
    RUN_TEST(ToggleFilter_ToggleInput_ChangesControlOutput);
    
    return UNITY_END();
}
