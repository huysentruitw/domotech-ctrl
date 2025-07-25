#include <unity.h>
#include <memory>

#include <DimmerFilter.h>

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void DimmerFilter_Constructor_CreatesPins()
{
    // Arrange
    DimmerFilter filter;

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
    TEST_ASSERT_EQUAL(DimmerControlValue(0, 2), controlPin->GetStateAs<DimmerControlValue>());
}

void DimmerFilter_ToggleInput_ChangesControlOutput()
{
    // Arrange
    DimmerFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto togglePin = inputPins[0].lock();
    auto controlPin = outputPins[0].lock();

    // Act: Set toggle input to true
    togglePin->SetState(DigitalValue(true));

    // Assert: Control output should now be set to 100% with 2 seconds fade time
    TEST_ASSERT_EQUAL(DimmerControlValue(100, 2), controlPin->GetStateAs<DimmerControlValue>());

    // Act: Set toggle input to false
    togglePin->SetState(DigitalValue(false));

    // Assert: Control output should remain at 100% with 2 seconds fade time
    TEST_ASSERT_EQUAL(DimmerControlValue(100, 2), controlPin->GetStateAs<DimmerControlValue>());

    // Act: Set toggle input to true again
    togglePin->SetState(DigitalValue(true));

    // Assert: Control output should now be set to 0% with 2 seconds fade time
    TEST_ASSERT_EQUAL(DimmerControlValue(0, 2), controlPin->GetStateAs<DimmerControlValue>());

    // Act: Set toggle input to false again
    togglePin->SetState(DigitalValue(false));

    // Assert: Control output should remain at 0% with 2 seconds fade time
    TEST_ASSERT_EQUAL(DimmerControlValue(0, 2), controlPin->GetStateAs<DimmerControlValue>());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(DimmerFilter_Constructor_CreatesPins);
    RUN_TEST(DimmerFilter_ToggleInput_ChangesControlOutput);
    
    return UNITY_END();
}
