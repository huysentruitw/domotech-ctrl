#include <memory>

#include <unity.h>

#include <Pins.h>

// Counters for callback testing
int g_callbackCounter = 0;
bool g_lastState = false;

void setUp(void)
{
    // Reset counters before each test
    g_callbackCounter = 0;
    g_lastState = false;
}

void tearDown(void)
{
    // Clean up after each test
}

// Callback function for testing InputPin
void onStateChangeCallback(bool newState)
{
    g_callbackCounter++;
    g_lastState = newState;
}

// Test that InputPin initial state works correctly
void InputPin_InitialState()
{
    // Arrange & Act
    auto pin = std::make_shared<InputPin<bool>>(onStateChangeCallback, true);
    
    // Assert
    TEST_ASSERT_TRUE(pin->GetState());
    TEST_ASSERT_EQUAL(0, g_callbackCounter); // Constructor shouldn't trigger callback
}

// Test that InputPin state change triggers callback
void InputPin_StateChangeTriggersCallback()
{
    // Arrange
    auto pin = std::make_shared<InputPin<bool>>(onStateChangeCallback, false);
    
    // Act
    pin->SetState(true);
    
    // Assert
    TEST_ASSERT_TRUE(pin->GetState());
    TEST_ASSERT_EQUAL(1, g_callbackCounter);
    TEST_ASSERT_TRUE(g_lastState);
    
    // Act again - same state should not trigger callback
    pin->SetState(true);
    
    // Assert
    TEST_ASSERT_EQUAL(1, g_callbackCounter); // No additional callback
}

// Test that OutputPin initial state works correctly
void OutputPin_InitialState()
{
    // Arrange & Act
    OutputPin<bool> pin(true);
    
    // Assert
    TEST_ASSERT_TRUE(pin.GetState());
}

// Test OutputPin state change
void OutputPin_StateChange()
{
    // Arrange
    OutputPin<bool> pin(false);
    
    // Act
    pin.SetState(true);
    
    // Assert
    TEST_ASSERT_TRUE(pin.GetState());
}

// Test connection between OutputPin and InputPin
void PinConnection_PropagatesInitialState()
{
    // Arrange
    auto outputPin = std::make_shared<OutputPin<bool>>(true);
    auto inputPin = std::make_shared<InputPin<bool>>(onStateChangeCallback, false);
    
    // Act
    bool result = inputPin->ConnectTo(outputPin);
    
    // Assert
    TEST_ASSERT_TRUE(result); // Connection successful
    TEST_ASSERT_TRUE(inputPin->GetState()); // Input state updated to match output
    TEST_ASSERT_EQUAL(1, g_callbackCounter); // Callback triggered once
}

// Test propagation of state changes from output to input
void PinConnection_PropagatesStateChanges()
{
    // Arrange
    auto outputPin = std::make_shared<OutputPin<bool>>(false);
    auto inputPin = std::make_shared<InputPin<bool>>(onStateChangeCallback, false);
    inputPin->ConnectTo(outputPin);
    g_callbackCounter = 0; // Reset after connection
    
    // Act
    outputPin->SetState(true);
    
    // Assert
    TEST_ASSERT_TRUE(inputPin->GetState());
    TEST_ASSERT_EQUAL(1, g_callbackCounter);
    TEST_ASSERT_TRUE(g_lastState);
}

// Test connection between one output and multiple inputs
void PinConnection_OneOutputToMultipleInputs()
{
    // Arrange
    auto outputPin = std::make_shared<OutputPin<bool>>(false);
    
    int counter1 = 0;
    bool state1 = false;
    auto inputPin1 = std::make_shared<InputPin<bool>>([&counter1, &state1](bool s) { counter1++; state1 = s; }, false);
    
    int counter2 = 0;
    bool state2 = false;
    auto inputPin2 = std::make_shared<InputPin<bool>>([&counter2, &state2](bool s) { counter2++; state2 = s; }, false);
    
    // Act
    inputPin1->ConnectTo(outputPin);
    inputPin2->ConnectTo(outputPin);
    counter1 = counter2 = 0; // Reset after connections
    
    outputPin->SetState(true);
    
    // Assert
    TEST_ASSERT_TRUE(inputPin1->GetState());
    TEST_ASSERT_TRUE(inputPin2->GetState());
    TEST_ASSERT_EQUAL(1, counter1);
    TEST_ASSERT_EQUAL(1, counter2);
    TEST_ASSERT_TRUE(state1);
    TEST_ASSERT_TRUE(state2);
}

// Test disconnection of pins
void PinConnection_Disconnect()
{
    // Arrange
    auto outputPin = std::make_shared<OutputPin<bool>>(true);
    auto inputPin = std::make_shared<InputPin<bool>>(onStateChangeCallback, false);
    inputPin->ConnectTo(outputPin);
    g_callbackCounter = 0; // Reset after connection
    
    // Act
    inputPin->Disconnect();
    outputPin->SetState(false); // Change state after disconnect
    
    // Assert
    TEST_ASSERT_FALSE(inputPin->GetState()); // Input should have reset to default state (false)
    TEST_ASSERT_EQUAL(1, g_callbackCounter); // Callback triggered by disconnect
}

// Test that expired output pins are handled correctly
void PinConnection_ExpiredOutputPin()
{
    // Arrange
    auto inputPin = std::make_shared<InputPin<bool>>(onStateChangeCallback, false);
    
    // Act
    bool result;
    {
        auto tempOutputPin = std::make_shared<OutputPin<bool>>(true);
        result = inputPin->ConnectTo(tempOutputPin);
    } // tempOutputPin goes out of scope here
    
    // Assert
    TEST_ASSERT_TRUE(result); // Connection was successful
    // Input state remains at last known value from output (true)
    TEST_ASSERT_TRUE(inputPin->GetState());
}

// Test that expired input pins are handled correctly
void PinConnection_ExpiredInputPin()
{
    // Arrange
    auto outputPin = std::make_shared<OutputPin<bool>>(false);
    
    // Act
    {
        auto tempInputPin = std::make_shared<InputPin<bool>>(onStateChangeCallback, false);
        tempInputPin->ConnectTo(outputPin);
    } // tempInputPin goes out of scope here
    
    // Clean up expired weak pointers by triggering a state change
    outputPin->SetState(true);
    
    // There's no explicit assertion here since we're mainly testing that
    // the code doesn't crash when handling expired weak pointers
    TEST_ASSERT_TRUE(true);
}

// Test connecting input pin to already connected pin
void PinConnection_AlreadyConnected()
{
    // Arrange
    auto outputPin1 = std::make_shared<OutputPin<bool>>(false);
    auto outputPin2 = std::make_shared<OutputPin<bool>>(true);
    auto inputPin = std::make_shared<InputPin<bool>>(onStateChangeCallback, false);
    
    // Act
    bool result1 = inputPin->ConnectTo(outputPin1);
    bool result2 = inputPin->ConnectTo(outputPin2); // Try to connect to second pin
    
    // Assert
    TEST_ASSERT_TRUE(result1); // First connection successful
    TEST_ASSERT_FALSE(result2); // Second connection fails
    TEST_ASSERT_FALSE(inputPin->GetState()); // State should be from first pin
}

// Main test runner
int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(InputPin_InitialState);
    RUN_TEST(InputPin_StateChangeTriggersCallback);
    RUN_TEST(OutputPin_InitialState);
    RUN_TEST(OutputPin_StateChange);
    RUN_TEST(PinConnection_PropagatesInitialState);
    RUN_TEST(PinConnection_PropagatesStateChanges);
    RUN_TEST(PinConnection_OneOutputToMultipleInputs);
    RUN_TEST(PinConnection_Disconnect);
    RUN_TEST(PinConnection_ExpiredOutputPin);
    RUN_TEST(PinConnection_ExpiredInputPin);
    RUN_TEST(PinConnection_AlreadyConnected);
    
    return UNITY_END();
}
