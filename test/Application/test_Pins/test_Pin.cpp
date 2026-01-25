#include <memory>

#include <unity.h>

#include <IPinObserver.h>
#include <Pin.h>
#include <PinFactory.h>

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Clean up after each test
}

// Callback function for testing InputPin
class StateObserver : public IPinObserver
{
public:
    int g_callbackCounter = 0;
    DigitalValue g_lastState = DigitalValue(false);

    void OnPinStateChanged(const Pin& pin) noexcept override
    {
        g_callbackCounter++;
        g_lastState = pin.GetStateAs<DigitalValue>();        
    }
};

void InputPin_InitialState()
{
    // Arrange & Act
    StateObserver observer;
    auto pin = PinFactory::CreateInputPin<DigitalValue>(&observer, DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), pin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(0, observer.g_callbackCounter); // Constructor shouldn't trigger callback
    TEST_ASSERT_FALSE(pin->IsConnected());
}

void InputPin_StateChange_TriggersCallback()
{
    // Arrange
    StateObserver observer;
    auto pin = PinFactory::CreateInputPin<DigitalValue>(&observer);
    
    // Act
    pin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), pin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(1, observer.g_callbackCounter);
    TEST_ASSERT_TRUE(observer.g_lastState);
    
    // Act again - same state should not trigger callback
    pin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(1, observer.g_callbackCounter); // No additional callback
}

void OutputPin_InitialState()
{
    // Arrange & Act
    auto pin = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), pin->GetStateAs<DigitalValue>());
    TEST_ASSERT_FALSE(pin->IsConnected());
}

void OutputPin_StateChange()
{
    // Arrange
    auto pin = PinFactory::CreateOutputPin<DigitalValue>();
    
    // Act
    pin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), pin->GetStateAs<DigitalValue>());
}

void PinConnection_PropagatesInitialState()
{
    // Arrange
    StateObserver observer;
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(&observer);
    
    // Act
    bool result = Pin::Connect(inputPin, outputPin);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(DigitalValue(true), inputPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(1, observer.g_callbackCounter);
}

void PinConnection_PropagatesStateChanges()
{
    // Arrange
    StateObserver observer;
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>();
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(&observer);
    Pin::Connect(inputPin, outputPin);
    observer.g_callbackCounter = 0; // Reset after connection
    
    // Act
    outputPin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), inputPin->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(1, observer.g_callbackCounter);
    TEST_ASSERT_EQUAL(DigitalValue(true), observer.g_lastState);
}

void PinConnection_OneOutputToMultipleInputs_AllInputsReceiveState()
{
    // Arrange
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>();
    
    StateObserver observer1;
    auto inputPin1 = PinFactory::CreateInputPin<DigitalValue>(&observer1);

    StateObserver observer2;
    auto inputPin2 = PinFactory::CreateInputPin<DigitalValue>(&observer2);
    
    // Act
    Pin::Connect(inputPin1, outputPin);
    Pin::Connect(inputPin2, outputPin);
    observer1.g_callbackCounter = observer2.g_callbackCounter = 0; // Reset after connections
    
    outputPin->SetState(DigitalValue(true));
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(true), inputPin1->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(DigitalValue(true), inputPin2->GetStateAs<DigitalValue>());
    TEST_ASSERT_EQUAL(1, observer1.g_callbackCounter);
    TEST_ASSERT_EQUAL(1, observer2.g_callbackCounter);
    TEST_ASSERT_EQUAL(DigitalValue(true), observer1.g_lastState);
    TEST_ASSERT_EQUAL(DigitalValue(true), observer2.g_lastState);
}

void PinConnection_Disconnect_InputResetsToDefaultState()
{
    // Arrange
    StateObserver observer;
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(&observer, DigitalValue(false));
    Pin::Connect(inputPin, outputPin);
    observer.g_callbackCounter = 0; // Reset after connection
    
    // Act
    Pin::Disconnect(inputPin, outputPin);
    outputPin->SetState(false); // Change state after disconnect
    
    // Assert
    TEST_ASSERT_EQUAL(DigitalValue(false), inputPin->GetStateAs<DigitalValue>()); // Input should have reset to default state (false)
    TEST_ASSERT_EQUAL(1, observer.g_callbackCounter); // Callback triggered by disconnect
}

void PinConnection_ExpiredOutputPin()
{
    // Arrange
    StateObserver observer;    
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(&observer);
    
    // Act
    bool result;
    {
        auto tempOutputPin = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
        result = Pin::Connect(inputPin, tempOutputPin);
        observer.g_callbackCounter = 0; // Reset after connection
    } // tempOutputPin goes out of scope here
    
    // Assert
    TEST_ASSERT_TRUE(result); // Connection was successful
    TEST_ASSERT_EQUAL(DigitalValue(false), inputPin->GetStateAs<DigitalValue>()); // Input state back to default
    TEST_ASSERT_EQUAL(1, observer.g_callbackCounter); // Callback triggered by connection
}

void PinConnection_ExpiredInputPin()
{
    // Arrange
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>();
    
    // Act
    {
        StateObserver observer;
        auto tempInputPin = PinFactory::CreateInputPin<DigitalValue>(&observer);
        Pin::Connect(tempInputPin, outputPin);
    } // tempInputPin goes out of scope here
    
    // Clean up expired weak pointers by triggering a state change
    outputPin->SetState(DigitalValue(true));
    
    // There's no explicit assertion here since we're mainly testing that
    // the code doesn't crash when handling expired weak pointers
    TEST_ASSERT_TRUE(true);
}

void PinConnection_AlreadyConnected_ConnectFails()
{
    // Arrange
    StateObserver observer;
    auto outputPin1 = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(false));
    auto outputPin2 = PinFactory::CreateOutputPin<DigitalValue>(DigitalValue(true));
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>(&observer);
    
    // Act
    bool result1 = Pin::Connect(inputPin, outputPin1);
    bool result2 = Pin::Connect(inputPin, outputPin2); // Try to connect to second pin
    
    // Assert
    TEST_ASSERT_TRUE(result1); // First connection successful
    TEST_ASSERT_FALSE(result2); // Second connection fails
    TEST_ASSERT_EQUAL(DigitalValue(false), inputPin->GetStateAs<DigitalValue>()); // State should be from first pin
}

void PinConnection_IsConnectedTrue()
{
    // Arrange
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>();
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>();

    // Act
    Pin::Connect(inputPin, outputPin);

    // Assert
    TEST_ASSERT_TRUE(inputPin->IsConnected());
    TEST_ASSERT_TRUE(outputPin->IsConnected());
}

void PinConnection_Disconnect_IsConnectedFalse()
{
    // Arrange
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>();
    auto outputPin = PinFactory::CreateOutputPin<DigitalValue>();
    Pin::Connect(inputPin, outputPin);

    // Act
    Pin::Disconnect(inputPin, outputPin);    

    // Assert
    TEST_ASSERT_FALSE(inputPin->IsConnected());
    TEST_ASSERT_FALSE(outputPin->IsConnected());
}

void PinConnection_ExpiredConnection_IsConnectedFalse()
{
    // Arrange
    auto inputPin = PinFactory::CreateInputPin<DigitalValue>();

    // Act
    {
        auto tempOutputPin = PinFactory::CreateOutputPin<DigitalValue>();
        Pin::Connect(inputPin, tempOutputPin);
    } // tempOutputPin goes out of scope here

    // Assert
    TEST_ASSERT_FALSE(inputPin->IsConnected());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(InputPin_InitialState);
    RUN_TEST(InputPin_StateChange_TriggersCallback);
    RUN_TEST(OutputPin_InitialState);
    RUN_TEST(OutputPin_StateChange);
    RUN_TEST(PinConnection_PropagatesInitialState);
    RUN_TEST(PinConnection_PropagatesStateChanges);
    RUN_TEST(PinConnection_OneOutputToMultipleInputs_AllInputsReceiveState);
    RUN_TEST(PinConnection_Disconnect_InputResetsToDefaultState);
    RUN_TEST(PinConnection_ExpiredOutputPin);
    RUN_TEST(PinConnection_ExpiredInputPin);
    RUN_TEST(PinConnection_AlreadyConnected_ConnectFails);
    RUN_TEST(PinConnection_IsConnectedTrue);
    RUN_TEST(PinConnection_Disconnect_IsConnectedFalse);
    RUN_TEST(PinConnection_ExpiredConnection_IsConnectedFalse);
    
    return UNITY_END();
}
