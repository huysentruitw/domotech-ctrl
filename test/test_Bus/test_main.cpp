#include <vector>
#include <unity.h>
#include <BusProtocol.h>

#include "../Fakes/FakeBusDriver.h"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_Poll_FlushesInput()
{
    // Arrange
    FakeBusState state;
    FakeBusDriver driver(state);
    BusProtocol bus(driver);

    // Act
    auto response = bus.Poll(0x01);

    // Assert
    TEST_ASSERT_TRUE(state.FlushInputCalled);
}

void test_Poll_Successful()
{
    // Arrange
    FakeBusState state;
    FakeBusDriver driver(state);
    BusProtocol bus(driver);
    uint8_t expectedBytesWritten[] = { 0xC4, 0xF1 };
    state.BytesToRead = { 0xD5 }; // Simulate a success response from the bus

    // Act
    auto response = bus.Poll(0x14, /*retries:*/ 0);

    // Assert
    TEST_ASSERT_EQUAL(2, state.BytesWritten.size());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedBytesWritten, state.BytesWritten.data(), 2);
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_FALSE(response.RespondedWithTypeAndData); // Poll does not respond with type and data
}

void test_Exchange_FlushesInput()
{
    // Arrange
    FakeBusState state;
    FakeBusDriver driver(state);
    BusProtocol bus(driver);

    // Act
    auto response = bus.Exchange(0x01, 0x1234);

    // Assert
    TEST_ASSERT_TRUE(state.FlushInputCalled);
}

void test_Exchange_Successful()
{
    // Arrange
    FakeBusState state;
    FakeBusDriver driver(state);
    BusProtocol bus(driver);
    uint8_t expectedBytesWritten[] = { 0xC4, 0xE1, 0x94, 0x93, 0x92, 0x91, 0xBC };
    state.BytesToRead = { 0x84, 0xB5, 0xA8, 0xA7, 0xA6, 0xA5, 0xBC };

    // Act
    auto response = bus.Exchange(0x14, 0x1234, /*retries:*/ 0);

    // Assert
    TEST_ASSERT_EQUAL(7, state.BytesWritten.size());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedBytesWritten, state.BytesWritten.data(), 7);
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_TRUE(response.RespondedWithTypeAndData); // Exchange responds with type and data
    TEST_ASSERT_EQUAL_UINT8(0x04, response.ModuleType); // Module type is 0x04
    TEST_ASSERT_EQUAL_UINT16(0x5678, response.Data); // Response data is 0x5678
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_Poll_FlushesInput);
    RUN_TEST(test_Poll_Successful);
    RUN_TEST(test_Exchange_FlushesInput);
    RUN_TEST(test_Exchange_Successful);

    return UNITY_END();
}