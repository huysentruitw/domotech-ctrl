#include <unity.h>

#include <ConnectionsParser.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void ConnectionsParser_EmptyString()
{
    // Arrange
    std::string connections = "";

    // Act
    auto result = TryParseConnections<4>(connections);

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, (*result).Count());
}

void ConnectionsParser_InvalidString()
{
    // Arrange
    std::string connections = "ABC";

    // Act
    auto result = TryParseConnections<4>(connections);

    // Assert
    TEST_ASSERT_FALSE(result);
}

void ConnectionsParser_SingleMapping()
{
    // Arrange
    std::string connections = "I1=A25:O3";
    Mapping expectedMapping =
    {
        .LocalPin =
        {
            .Direction = PinDirection::Input,
            .Index = 1,
        },
        .RemoteModule =
        {
            .Address = 25,
        },
        .RemotePin =
        {
            .Direction = PinDirection::Output,
            .Index = 3,
        },
    };

    // Act
    auto result = TryParseConnections<4>(connections);

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, (*result).Count());

    const auto mapping = (*result)[0];
    TEST_ASSERT_EQUAL(expectedMapping.LocalPin.Direction, mapping.LocalPin.Direction);
    TEST_ASSERT_EQUAL(expectedMapping.LocalPin.Index, mapping.LocalPin.Index);
    TEST_ASSERT_EQUAL(expectedMapping.RemoteModule.Address, mapping.RemoteModule.Address);
    TEST_ASSERT_EQUAL(expectedMapping.RemotePin.Direction, mapping.RemotePin.Direction);
    TEST_ASSERT_EQUAL(expectedMapping.RemotePin.Index, mapping.RemotePin.Index);
}

void ConnectionsParser_MultipleMapping()
{
    // Arrange
    std::string connections = "I0=A3:O7,O0=A5:I7,I1=A6:O7";

    // Act
    auto result = TryParseConnections<4>(connections);

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(3, (*result).Count());

    TEST_ASSERT_EQUAL(3, (*result)[0].RemoteModule.Address);
    TEST_ASSERT_EQUAL(5, (*result)[1].RemoteModule.Address);
    TEST_ASSERT_EQUAL(6, (*result)[2].RemoteModule.Address);
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(ConnectionsParser_EmptyString);
    RUN_TEST(ConnectionsParser_InvalidString);
    RUN_TEST(ConnectionsParser_SingleMapping);
    RUN_TEST(ConnectionsParser_MultipleMapping);

    return UNITY_END();
}
