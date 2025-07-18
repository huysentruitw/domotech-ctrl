#include <unity.h>

#include <vector>

#include <Bus.h>
#include "../../Mocks/MockBusDriver.h"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void Poll_FlushesInput()
{
    // Arrange
    MockBusDriver driver;
    Bus bus(driver);

    // Act
    auto response = bus.Poll(0x01);

    // Assert
    TEST_ASSERT_TRUE(driver.FlushInputCalled);
}

void Poll_Successful()
{
    // Arrange
    MockBusDriver driver;
    Bus bus(driver);
    uint8_t expectedBytesWritten[] = { 0xC4, 0xF1 };
    driver.BytesToRead = { 0xD5 }; // Simulate a success response from the bus

    // Act
    auto response = bus.Poll(0x14, /*retries:*/ 0);

    // Assert
    TEST_ASSERT_EQUAL(2, driver.BytesWritten.size());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedBytesWritten, driver.BytesWritten.data(), 2);
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_FALSE(response.RespondedWithTypeAndData); // Poll does not respond with type and data
}

void Exchange_FlushesInput()
{
    // Arrange
    MockBusDriver driver;
    Bus bus(driver);

    // Act
    auto response = bus.Exchange(0x01, 0x1234);

    // Assert
    TEST_ASSERT_TRUE(driver.FlushInputCalled);
}

void Exchange_Successful()
{
    // Arrange
    MockBusDriver driver;
    Bus bus(driver);
    uint8_t expectedBytesWritten[] = { 0xC4, 0xE1, 0x94, 0x93, 0x92, 0x91, 0xBC };
    driver.BytesToRead = { 0x84, 0xB5, 0xA8, 0xA7, 0xA6, 0xA5, 0xBC };

    // Act
    auto response = bus.Exchange(0x14, 0x1234, /*retries:*/ 0);

    // Assert
    TEST_ASSERT_EQUAL(7, driver.BytesWritten.size());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedBytesWritten, driver.BytesWritten.data(), 7);
    TEST_ASSERT_TRUE(response.Success);
    TEST_ASSERT_TRUE(response.RespondedWithTypeAndData); // Exchange responds with type and data
    TEST_ASSERT_EQUAL_UINT8(0x04, response.ModuleType); // Module type is 0x04
    TEST_ASSERT_EQUAL_UINT16(0x5678, response.Data); // Response data is 0x5678
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(Poll_FlushesInput);
    RUN_TEST(Poll_Successful);
    RUN_TEST(Exchange_FlushesInput);
    RUN_TEST(Exchange_Successful);

    return UNITY_END();
}