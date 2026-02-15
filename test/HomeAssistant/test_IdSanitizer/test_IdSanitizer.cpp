#include <unity.h>

#include <IdSanitizer.h>

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void IdSanitizer_ValidCharacters_ReturnsUnchanged()
{
    TEST_ASSERT_EQUAL_STRING("valid-id_123", IdSanitizer::Sanitize("valid-id_123").c_str());
}

void IdSanitizer_UppercaseAndSpaces_ConvertsAndNormalizes()
{
    TEST_ASSERT_EQUAL_STRING("my-test-id", IdSanitizer::Sanitize("My Test ID").c_str());
}

void IdSanitizer_SpecialCharacters_Removed()
{
    TEST_ASSERT_EQUAL_STRING("mytestid123", IdSanitizer::Sanitize("my@test#id$123").c_str());
}

void IdSanitizer_ExceedsMaxLength_Truncates()
{
    std::string result = IdSanitizer::Sanitize("this_is_a_very_long_id_that_exceeds_the_maximum_allowed_length");
    TEST_ASSERT_EQUAL(32, result.length());
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(IdSanitizer_ValidCharacters_ReturnsUnchanged);
    RUN_TEST(IdSanitizer_UppercaseAndSpaces_ConvertsAndNormalizes);
    RUN_TEST(IdSanitizer_SpecialCharacters_Removed);
    RUN_TEST(IdSanitizer_ExceedsMaxLength_Truncates);

    return UNITY_END();
}
