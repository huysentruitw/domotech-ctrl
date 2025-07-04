#include <unity.h>
#include <BusProtocol.h>

class MockBusDriver : public BusDriver
{
    void Init() {}
    void WriteBytes(const uint8_t* data, const uint16_t len) {}
    bool ReadBytes(uint8_t* data, const uint16_t len) { return true; }
    void FlushInput() {}
};

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_Poll()
{
    // Arrange
    MockBusDriver mockDriver;
    BusProtocol bus(mockDriver);

    // // Act
    // auto response = bus.Poll(0x01);

    // // Assert
    // TEST_ASSERT_TRUE(response.Success);
    // TEST_ASSERT_TRUE(response.RespondedWithTypeAndData);
    // TEST_ASSERT_EQUAL(ModuleType::PushButtons, response.ModuleType);
    // TEST_ASSERT_EQUAL(0x1234, response.Data);
}

int main( int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_Poll);

    UNITY_END();
}