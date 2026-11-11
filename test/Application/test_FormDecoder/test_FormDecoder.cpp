#include <unity.h>
#include <FormDecoder.h>

void setUp(void) {}
void tearDown(void) {}

void TryGetValue_PlainValue()
{
    char out[32];
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue("wifi_ssid=MyNetwork&wifi_pass=secret", "wifi_ssid", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("MyNetwork", out);
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue("wifi_ssid=MyNetwork&wifi_pass=secret", "wifi_pass", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("secret", out);
}

void TryGetValue_DecodesPlusAndPercentInEveryField()
{
    char out[64];
    const char* body = "wifi_ssid=My+Home+Net&wifi_pass=a+b%26c%3Dd%25&ha_mqtt_uri=mqtt%3A%2F%2F192.168.0.2";

    TEST_ASSERT_TRUE(FormDecoder::TryGetValue(body, "wifi_ssid", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("My Home Net", out);
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue(body, "wifi_pass", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("a b&c=d%", out);
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue(body, "ha_mqtt_uri", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("mqtt://192.168.0.2", out);
}

void TryGetValue_DecodesLowercaseHexAndUtf8Bytes()
{
    char out[32];
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue("k=%c3%a9t%C3%A9", "k", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("\xC3\xA9t\xC3\xA9", out);
}

void TryGetValue_InvalidPercentSequence_IsCopiedLiterally()
{
    char out[32];
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue("k=100%zz%4", "k", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("100%zz%4", out);
}

void TryGetValue_EmptyValue_IsAllowed()
{
    char out[32] = "garbage";
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue("wifi_pass=&wifi_ssid=x", "wifi_pass", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("", out);
}

void TryGetValue_MissingKey_ReturnsFalse()
{
    char out[32];
    TEST_ASSERT_FALSE(FormDecoder::TryGetValue("wifi_ssid=x", "wifi_pass", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("", out);
}

void TryGetValue_MatchesWholeKeyOnly()
{
    char out[32];
    // "pass" must not match "wifi_pass", and a value containing "pass=" must not match either
    TEST_ASSERT_FALSE(FormDecoder::TryGetValue("wifi_pass=1&x=pass%3D2", "pass", out, sizeof(out)));
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue("x_ha_mqtt_pass=1&ha_mqtt_pass=2", "ha_mqtt_pass", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("2", out);
}

void TryGetValue_TooLongForBuffer_ReturnsFalse()
{
    char out[4];
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue("k=abc", "k", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("abc", out);
    TEST_ASSERT_FALSE(FormDecoder::TryGetValue("k=abcd", "k", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("", out);
}

void TryGetValue_LengthIsCheckedAfterDecoding()
{
    char out[4];
    // 9 encoded chars decode to 3 bytes, which fit
    TEST_ASSERT_TRUE(FormDecoder::TryGetValue("k=%41%42%43", "k", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("ABC", out);
}

void TryGetValue_EncodedNul_ReturnsFalse()
{
    char out[32];
    TEST_ASSERT_FALSE(FormDecoder::TryGetValue("k=ab%00cd", "k", out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("", out);
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();

    RUN_TEST(TryGetValue_PlainValue);
    RUN_TEST(TryGetValue_DecodesPlusAndPercentInEveryField);
    RUN_TEST(TryGetValue_DecodesLowercaseHexAndUtf8Bytes);
    RUN_TEST(TryGetValue_InvalidPercentSequence_IsCopiedLiterally);
    RUN_TEST(TryGetValue_EmptyValue_IsAllowed);
    RUN_TEST(TryGetValue_MissingKey_ReturnsFalse);
    RUN_TEST(TryGetValue_MatchesWholeKeyOnly);
    RUN_TEST(TryGetValue_TooLongForBuffer_ReturnsFalse);
    RUN_TEST(TryGetValue_LengthIsCheckedAfterDecoding);
    RUN_TEST(TryGetValue_EncodedNul_ReturnsFalse);

    return UNITY_END();
}
