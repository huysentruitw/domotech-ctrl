#include <unity.h>
#include <NumberUtilities.h>

void setUp(void) {}
void tearDown(void) {}

// ToHex4 tests
void ToHex4_MinMax()
{
    TEST_ASSERT_EQUAL_STRING("0000", NumberUtilities::ToHex4(0x0000).c_str());
    TEST_ASSERT_EQUAL_STRING("FFFF", NumberUtilities::ToHex4(0xFFFF).c_str());
    TEST_ASSERT_EQUAL_STRING("DEAD", NumberUtilities::ToHex4(0xDEAD).c_str());
}

// ParseInt base 10
void ParseInt_Base10()
{
    TEST_ASSERT_TRUE(NumberUtilities::ParseInt("0").has_value());
    TEST_ASSERT_EQUAL_INT(0, NumberUtilities::ParseInt("0").value());
    TEST_ASSERT_EQUAL_INT(42, NumberUtilities::ParseInt("42").value());
    TEST_ASSERT_EQUAL_INT(-42, NumberUtilities::ParseInt("-42").value());
    TEST_ASSERT_EQUAL_INT(42, NumberUtilities::ParseInt("+42").value());
}

// ParseInt base 16
void ParseInt_Base16()
{
    TEST_ASSERT_EQUAL_INT(255, NumberUtilities::ParseInt("FF", 16).value());
    TEST_ASSERT_EQUAL_INT(255, NumberUtilities::ParseInt("ff", 16).value());
    TEST_ASSERT_EQUAL_INT(0xDEAD, NumberUtilities::ParseInt("DEAD", 16).value());
    TEST_ASSERT_EQUAL_INT(-255, NumberUtilities::ParseInt("-FF", 16).value());
}

// ParseInt base 2
void ParseInt_Base2()
{
    TEST_ASSERT_EQUAL_INT(15, NumberUtilities::ParseInt("1111", 2).value());
    TEST_ASSERT_EQUAL_INT(0xAA, NumberUtilities::ParseInt("10101010", 2).value());
}

// ParseInt invalid
void ParseInt_Invalid()
{
    TEST_ASSERT_FALSE(NumberUtilities::ParseInt("").has_value());
    TEST_ASSERT_FALSE(NumberUtilities::ParseInt("+").has_value());
    TEST_ASSERT_FALSE(NumberUtilities::ParseInt("-").has_value());
    TEST_ASSERT_FALSE(NumberUtilities::ParseInt("12X5").has_value());
    TEST_ASSERT_FALSE(NumberUtilities::ParseInt("FF", 10).has_value());
    TEST_ASSERT_FALSE(NumberUtilities::ParseInt("1012", 2).has_value());
    TEST_ASSERT_FALSE(NumberUtilities::ParseInt("99999999999999999999").has_value());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(ToHex4_MinMax);
    RUN_TEST(ParseInt_Base10);
    RUN_TEST(ParseInt_Base16);
    RUN_TEST(ParseInt_Base2);
    RUN_TEST(ParseInt_Invalid);
    
    return UNITY_END();
}
