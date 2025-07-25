#include <memory>

#include <unity.h>

#include <Pin.h>
#include <PinFactory.h>

// Counters for callback testing
int g_callbackCounter = 0;
DigitalValue g_lastState = DigitalValue(false);

void setUp(void)
{
    // Reset counters before each test
    g_callbackCounter = 0;
    g_lastState = DigitalValue(false);
}

void tearDown(void)
{
    // Clean up after each test
}

// Callback function for testing InputPin
void onStateChangeCallback(const Pin& pin)
{
    g_callbackCounter++;
    g_lastState = pin.GetStateAs<DigitalValue>();
}

// Test that InputPin initial state works correctly
void InputPin_InitialState()
{
    // Arrange & Act
    auto pin = PinFactory::CreateInputPin<DigitalValue>(onStateChangeCallback, DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), pin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(0, g_callbackCounter); // Constructor shouldn't trigger callback
}

// Test that InputPin state change triggers callback
void InputPin_StateChangeTriggersCallback()
{
    // Arrange
    auto pin = PinFactory::CreateInputPin<DigitalValue>(onStateChangeCallback);
    
    // Act
    pin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), pin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(1, g_callbackCounter);
    TEST_ASSERT_TRUE(g_lastState);
    
    // Act again - same state should not trigger callback
    pin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(1, g_callbackCounter); // No additional callback
}

// Test that OutputPin initial state works correctly
void OutputPin_InitialState()
{
    // Arrange & Act
    auto pin = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), pin->GetStateAs<DigitalValue>());
}

// Test OutputPin state change
void OutputPin_StateChange()
{
    // Arrange
    auto pin = PinFactory::CreateOutputPin<DigitalValue>();
    
    // Act
    pin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), pin->GetStateAs<DigitalValue>());
}

// Test connection between OutputPin and InputPin
void PinConnection_PropagatesInitialState()
{
    // Arrange
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(onStateChangeCallback);
    
    // Act
    bool result = Pin::Connect(inputPin, outputPin);
    
    // Assert
    TEST_ASSERT_TRUE(result); // Connection successful
    TEST_ASSERT_EQUAL(DigitalValue(true), inputPin->GetStateAs<DigitalValue>()); // Input state updated to match output
    TEST_ASSERT_EQUAL(1, g_callbackCounter); // Callback triggered once
}

// Test propagation of state changes from output to input
void PinConnection_PropagatesStateChanges()
{
    // Arrange
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>();
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(onStateChangeCallback);
    Pin::Connect(inputPin, outputPin);
    g_callbackCounter = 0; // Reset after connection
    
    // Act
    outputPin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), inputPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(1, g_callbackCounter);
    TEST_ASSERT_EQUAL(DigitalValue(true), g_lastState);
}

// Test connection between one output and multiple inputs
void PinConnection_OneOutputToMultipleInputs()
{
    // Arrange
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>();
    
    int counter1 = 0;
    auto state1 = DigitalValue(false);
    auto inputPin1 = PinFactory::CreateInputPin<DigitalValue>([&counter1, &state1](const Pin& pin) { counter1++; state1 = pin.GetStateAs<DigitalValue>(); });
    
    int counter2 = 0;
    auto state2 = DigitalValue(false);
    auto inputPin2 = PinFactory::CreateInputPin<DigitalValue>([&counter2, &state2](const Pin& pin) { counter2++; state2 = pin.GetStateAs<DigitalValue>(); });
    
    // Act
    Pin::Connect(inputPin1, outputPin);
    Pin::Connect(inputPin2, outputPin);
    counter1 = counter2 = 0; // Reset after connections
    
    outputPin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), inputPin1->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(true), inputPin2->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(1, counter1);
    TEST_ASSERT_EQUAL(1, counter2);
    TEST_ASSERT_EQUAL(DigitalValue(true), state1);
    TEST_ASSERT_EQUAL(DigitalValue(true), state2);
}

// Test disconnection of pins
void PinConnection_Disconnect()
{
    // Arrange
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(onStateChangeCallback, DigitalValue(false));
    Pin::Connect(inputPin, outputPin);
    g_callbackCounter = 0; // Reset after connection
    
    // Act
    Pin::Disconnect(inputPin, outputPin);
    outputPin->SetState(false); // Change state after disconnect
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(false), inputPin->GetStateAs<DigitalValue>()); // Input should have reset to default state (false)
    TEST_ASSERT_EQUAL(1, g_callbackCounter); // Callback triggered by disconnect
}

// Test that expired output pins are handled correctly
void PinConnection_ExpiredOutputPin()
{
    // Arrange
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(onStateChangeCallback);
    
    // Act
    bool result;
    {
        auto tempOutputPin = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
        result = Pin::Connect(inputPin, tempOutputPin);
    } // tempOutputPin goes out of scope here
    
    // Assert
    TEST_ASSERT_TRUE(result); // Connection was successful
    // Input state remains at last known value from output (true)
    TEST_ASSERT_EQUAL(DigitalValue(true), inputPin->GetStateAs<DigitalValue>());
}

// Test that expired input pins are handled correctly
void PinConnection_ExpiredInputPin()
{
    // Arrange
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>();
    
    // Act
    {
        auto tempInputPin = PinFactory::CreateInputPin<DigitalValue>(onStateChangeCallback);
        Pin::Connect(tempInputPin, outputPin);
    } // tempInputPin goes out of scope here
    
    // Clean up expired weak pointers by triggering a state change
    outputPin->SetState(DigitalValue(true));
    
    // There's no explicit assertion here since we're mainly testing that
    // the code doesn't crash when handling expired weak pointers
    TEST_ASSERT_TRUE(true);
}

// Test connecting input pin to already connected pin
void PinConnection_AlreadyConnected()
{
    // Arrange
    auto outputPin1 = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(false));
    auto outputPin2 = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(onStateChangeCallback);
    
    // Act
    bool result1 = Pin::Connect(inputPin, outputPin1);
    bool result2 = Pin::Connect(inputPin, outputPin2); // Try to connect to second pin
    
    // Assert
    TEST_ASSERT_TRUE(result1); // First connection successful
    TEST_ASSERT_FALSE(result2); // Second connection fails
    TEST_ASSERT_EQUAL(DigitalValue(false), inputPin->GetStateAs<DigitalValue>()); // State should be from first pin
}

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
