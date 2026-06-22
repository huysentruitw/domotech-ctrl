#include <unity.h>
#include <memory>

#include <Filters/DimmerFilter.h>

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
    TEST_ASSERT_EQUAL(2, outputPins.size());

    auto togglePin = inputPins[0].lock();
    auto controlPin = outputPins[0].lock();
    auto feedbackPin = outputPins[1].lock();

    TEST_ASSERT_NOT_NULL(togglePin.get());
    TEST_ASSERT_NOT_NULL(controlPin.get());
    TEST_ASSERT_NOT_NULL(feedbackPin.get());

    TEST_ASSERT_EQUAL(DigitalValue(false), togglePin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DimmerControlValue(0, 2), controlPin->GetStateAs<DimmerControlValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), feedbackPin->GetStateAs<DigitalValue>());
}

void DimmerFilter_ToggleInput_ChangesControlOutput()
{
    // Arrange
    DimmerFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto togglePin = inputPins[0].lock();
    auto controlPin = outputPins[0].lock();
    auto feedbackPin = outputPins[1].lock();

    // Act: Short press (true then false quickly, simulating < 300ms press)
    togglePin->SetState(DigitalValue(true));
    togglePin->SetState(DigitalValue(false));

    // Assert: After short press release, should toggle to 100% with 2 seconds fade time
    TEST_ASSERT_EQUAL(DimmerControlValue(100, 2), controlPin->GetStateAs<DimmerControlValue>());
    TEST_ASSERT_EQUAL(DigitalValue(true), feedbackPin->GetStateAs<DigitalValue>());

    // Act: Another short press
    togglePin->SetState(DigitalValue(true));
    togglePin->SetState(DigitalValue(false));

    // Assert: Should toggle back to 0% with 2 seconds fade time
    TEST_ASSERT_EQUAL(DimmerControlValue(0, 2), controlPin->GetStateAs<DimmerControlValue>());
    TEST_ASSERT_EQUAL(DigitalValue(false), feedbackPin->GetStateAs<DigitalValue>());
}

void DimmerFilter_LongPress_DimmsUpward()
{
    // Arrange
    DimmerFilter filter;
    auto inputPins = filter.GetInputPins();
    auto outputPins = filter.GetOutputPins();

    auto togglePin = inputPins[0].lock();
    auto controlPin = outputPins[0].lock();

    // Start dimmer at 50%
    filter.SetState(DimmerControlValue(50, 0));
    TEST_ASSERT_EQUAL(50, controlPin->GetStateAs<DimmerControlValue>().GetPercentage());

    // Act: Long press - button press
    togglePin->SetState(DigitalValue(true));

    // Simulate button held - call Every100Ms through Filter interface to increment press duration
    // First call: duration = 1 (100ms)
    static_cast<Filter&>(filter).Every100Ms();
    TEST_ASSERT_EQUAL(50, controlPin->GetStateAs<DimmerControlValue>().GetPercentage());

    // Second call: duration = 2 (200ms)
    static_cast<Filter&>(filter).Every100Ms();
    TEST_ASSERT_EQUAL(50, controlPin->GetStateAs<DimmerControlValue>().GetPercentage());

    // Third call: duration = 3 (300ms) - should start dimming
    static_cast<Filter&>(filter).Every100Ms();
    TEST_ASSERT_EQUAL(51, controlPin->GetStateAs<DimmerControlValue>().GetPercentage());

    // Continue dimming up while button is held
    static_cast<Filter&>(filter).Every100Ms();
    TEST_ASSERT_EQUAL(52, controlPin->GetStateAs<DimmerControlValue>().GetPercentage());

    static_cast<Filter&>(filter).Every100Ms();
    TEST_ASSERT_EQUAL(53, controlPin->GetStateAs<DimmerControlValue>().GetPercentage());

    // Act: Button released after long press
    togglePin->SetState(DigitalValue(false));

    // Assert: Brightness should remain at 53%, dimming should stop
    TEST_ASSERT_EQUAL(53, controlPin->GetStateAs<DimmerControlValue>().GetPercentage());

    // Verify no further dimming after release
    static_cast<Filter&>(filter).Every100Ms();
    TEST_ASSERT_EQUAL(53, controlPin->GetStateAs<DimmerControlValue>().GetPercentage());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(DimmerFilter_Constructor_CreatesPins);
    RUN_TEST(DimmerFilter_ToggleInput_ChangesControlOutput);
    RUN_TEST(DimmerFilter_LongPress_DimmsUpward);
    
    return UNITY_END();
}
