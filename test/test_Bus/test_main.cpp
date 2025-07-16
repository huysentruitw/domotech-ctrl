#include <unity.h>
#include <BusProtocol.h>
#include <vector>
#include <cstring>
#include <algorithm>

class MockBusDriver : public BusDriver
{
public:
    void Init() override {}
    void WriteBytes(const uint8_t* data, const uint16_t len) override {}
    bool ReadBytes(uint8_t* data, const uint16_t len) override { return false; }
    void FlushInput() override {}
};

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_Basic()
{
    MockBusDriver mockDriver;
    BusProtocol busProtocol(mockDriver);

    // Simple test to verify Unity is working
    TEST_ASSERT_TRUE(true);
    TEST_ASSERT_EQUAL(42, 42);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_Basic);
    // RUN_TEST(test_Poll);

    return UNITY_END();
}