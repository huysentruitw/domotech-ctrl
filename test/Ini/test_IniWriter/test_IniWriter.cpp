#include <unity.h>
#include <IniWriter.h>
#include <string>

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_IniWriter_EmptyContent()
{
    // Arrange
    IniWriter writer;

    // Act
    std::string content = writer.GetContent();

    // Assert
    TEST_ASSERT_EQUAL_STRING("", content.c_str());
}

void test_IniWriter_SingleSection()
{
    // Arrange
    IniWriter writer;

    // Act
    writer.WriteSection("Section1");
    std::string content = writer.GetContent();

    // Assert
    TEST_ASSERT_EQUAL_STRING("[Section1]\n", content.c_str());
}

void test_IniWriter_SectionWithKeyValue()
{
    // Arrange
    IniWriter writer;

    // Act
    writer.WriteSection("Section1");
    writer.WriteKeyValue("Key1", "Value1");
    std::string content = writer.GetContent();

    // Assert
    const char* expected = "[Section1]\nKey1=Value1\n";
    TEST_ASSERT_EQUAL_STRING(expected, content.c_str());
}

void test_IniWriter_MultipleSections()
{
    // Arrange
    IniWriter writer;

    // Act
    writer.WriteSection("Section1");
    writer.WriteKeyValue("Key1", "Value1");
    writer.WriteSection("Section2");
    writer.WriteKeyValue("Key2", "Value2");
    std::string content = writer.GetContent();

    // Assert
    const char* expected = 
        "[Section1]\n"
        "Key1=Value1\n"
        "\n"
        "[Section2]\n"
        "Key2=Value2\n";
    TEST_ASSERT_EQUAL_STRING(expected, content.c_str());
}

void test_IniWriter_Comments()
{
    // Arrange
    IniWriter writer;

    // Act
    writer.WriteComment("This is a comment");
    writer.WriteSection("Section1");
    writer.WriteKeyValue("Key1", "Value1");
    writer.WriteComment("Another comment");
    writer.WriteKeyValue("Key2", "Value2");
    std::string content = writer.GetContent();

    // Assert
    const char* expected = 
        "; This is a comment\n"
        "[Section1]\n"
        "Key1=Value1\n"
        "; Another comment\n"
        "Key2=Value2\n";
    TEST_ASSERT_EQUAL_STRING(expected, content.c_str());
}

void test_IniWriter_MultipleKeyValues()
{
    // Arrange
    IniWriter writer;

    // Act
    writer.WriteSection("Section1");
    writer.WriteKeyValue("Key1", "Value1");
    writer.WriteKeyValue("Key2", "Value2");
    writer.WriteKeyValue("Key3", "Value3");
    std::string content = writer.GetContent();

    // Assert
    const char* expected = 
        "[Section1]\n"
        "Key1=Value1\n"
        "Key2=Value2\n"
        "Key3=Value3\n";
    TEST_ASSERT_EQUAL_STRING(expected, content.c_str());
}

void test_IniWriter_ComplexDocument()
{
    // Arrange
    IniWriter writer;

    // Act
    writer.WriteComment("Configuration file");
    writer.WriteSection("Database");
    writer.WriteKeyValue("Host", "localhost");
    writer.WriteKeyValue("Port", "5432");
    writer.WriteKeyValue("Username", "admin");
    writer.WriteComment("Security section");
    writer.WriteSection("Security");
    writer.WriteKeyValue("EnableSSL", "true");
    writer.WriteKeyValue("Timeout", "3600");
    std::string content = writer.GetContent();

    // Assert
    const char* expected = 
        "; Configuration file\n"
        "[Database]\n"
        "Host=localhost\n"
        "Port=5432\n"
        "Username=admin\n"
        "; Security section\n"
        "\n"
        "[Security]\n"
        "EnableSSL=true\n"
        "Timeout=3600\n";
    TEST_ASSERT_EQUAL_STRING(expected, content.c_str());
}

int main(void)
{
    UNITY_BEGIN();
    
    RUN_TEST(test_IniWriter_EmptyContent);
    RUN_TEST(test_IniWriter_SingleSection);
    RUN_TEST(test_IniWriter_SectionWithKeyValue);
    RUN_TEST(test_IniWriter_MultipleSections);
    RUN_TEST(test_IniWriter_Comments);
    RUN_TEST(test_IniWriter_MultipleKeyValues);
    RUN_TEST(test_IniWriter_ComplexDocument);

    return UNITY_END();
}
