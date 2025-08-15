#include <unity.h>

#include <IniReader.h>

#include <vector>

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_IniReader_EmptyContent()
{
    // Arrange
    IniReader reader("");
    int sectionCallCount = 0;
    int keyValueCallCount = 0;

    // Act
    reader.Process(
        [&](const std::string&) { sectionCallCount++; },
        [&](const std::string&, const std::string&, const std::string&) { keyValueCallCount++; });

    // Assert
    TEST_ASSERT_EQUAL(0, sectionCallCount);
    TEST_ASSERT_EQUAL(0, keyValueCallCount);
}

void test_IniReader_SingleSection()
{
    // Arrange
    IniReader reader("[Section1]\nKey1=Value1");
    std::string lastSection;
    std::vector<std::pair<std::string, std::string>> keyValues;

    // Act
    reader.Process(
        [&](const std::string& section) { lastSection = section; },
        [&](const std::string&, const std::string& key, const std::string& value) {
            keyValues.push_back({key, value});
        });

    // Assert
    TEST_ASSERT_EQUAL_STRING("Section1", lastSection.c_str());
    TEST_ASSERT_EQUAL(1, keyValues.size());
    TEST_ASSERT_EQUAL_STRING("Key1", keyValues[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("Value1", keyValues[0].second.c_str());
}

void test_IniReader_MultipleSections()
{
    // Arrange
    const char* content = R"(
[Section1]
Key1=Value1
Key2=Value2

[Section2]
Key3=Value3
    )";
    
    IniReader reader(content);
    std::vector<std::string> sections;
    std::vector<std::tuple<std::string, std::string, std::string>> keyValues;

    // Act
    reader.Process(
        [&](const std::string& section) { sections.push_back(section); },
        [&](const std::string& section, const std::string& key, const std::string& value) {
            keyValues.push_back({section, key, value});
        });

    // Assert
    TEST_ASSERT_EQUAL(2, sections.size());
    TEST_ASSERT_EQUAL_STRING("Section1", sections[0].c_str());
    TEST_ASSERT_EQUAL_STRING("Section2", sections[1].c_str());
    TEST_ASSERT_EQUAL(3, keyValues.size());
    TEST_ASSERT_EQUAL_STRING("Section1", std::get<0>(keyValues[0]).c_str());
    TEST_ASSERT_EQUAL_STRING("Key1", std::get<1>(keyValues[0]).c_str());
    TEST_ASSERT_EQUAL_STRING("Value1", std::get<2>(keyValues[0]).c_str());
    TEST_ASSERT_EQUAL_STRING("Section1", std::get<0>(keyValues[1]).c_str());
    TEST_ASSERT_EQUAL_STRING("Key2", std::get<1>(keyValues[1]).c_str());
    TEST_ASSERT_EQUAL_STRING("Value2", std::get<2>(keyValues[1]).c_str());
    TEST_ASSERT_EQUAL_STRING("Section2", std::get<0>(keyValues[2]).c_str());
    TEST_ASSERT_EQUAL_STRING("Key3", std::get<1>(keyValues[2]).c_str());
    TEST_ASSERT_EQUAL_STRING("Value3", std::get<2>(keyValues[2]).c_str());
}

void test_IniReader_IgnoresComments()
{
    // Arrange
    const char* content = R"(
; Comment at start
[Section1]
Key1=Value1
; Comment between values
Key2=Value2
    )";
    
    IniReader reader(content);
    int sectionCallCount = 0;
    std::vector<std::pair<std::string, std::string>> keyValues;

    // Act
    reader.Process(
        [&](const std::string&) { sectionCallCount++; },
        [&](const std::string&, const std::string& key, const std::string& value) {
            keyValues.push_back({key, value});
        });

    // Assert
    TEST_ASSERT_EQUAL(1, sectionCallCount);
    TEST_ASSERT_EQUAL(2, keyValues.size());
}

void test_IniReader_IgnoresEmptyLines()
{
    // Arrange
    const char* content = R"(

[Section1]

Key1=Value1

Key2=Value2

    )";
    
    IniReader reader(content);
    int sectionCallCount = 0;
    std::vector<std::pair<std::string, std::string>> keyValues;

    // Act
    reader.Process(
        [&](const std::string&) { sectionCallCount++; },
        [&](const std::string&, const std::string& key, const std::string& value) {
            keyValues.push_back({key, value});
        });

    // Assert
    TEST_ASSERT_EQUAL(1, sectionCallCount);
    TEST_ASSERT_EQUAL(2, keyValues.size());
}

void test_IniReader_HandlesWhitespace()
{
    // Arrange
    const char* content = R"(
   [  Section1  ]
  Key1  =  Value1  
    Key2=Value2    
    )";
    
    IniReader reader(content);
    std::string lastSection;
    std::vector<std::pair<std::string, std::string>> keyValues;

    // Act
    reader.Process(
        [&](const std::string& section) { lastSection = section; },
        [&](const std::string&, const std::string& key, const std::string& value) {
            keyValues.push_back({key, value});
        });

    // Assert
    TEST_ASSERT_EQUAL_STRING("Section1", lastSection.c_str());
    TEST_ASSERT_EQUAL(2, keyValues.size());
    TEST_ASSERT_EQUAL_STRING("Key1", keyValues[0].first.c_str());
    TEST_ASSERT_EQUAL_STRING("Value1", keyValues[0].second.c_str());
    TEST_ASSERT_EQUAL_STRING("Key2", keyValues[1].first.c_str());
    TEST_ASSERT_EQUAL_STRING("Value2", keyValues[1].second.c_str());
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_IniReader_EmptyContent);
    RUN_TEST(test_IniReader_SingleSection);
    RUN_TEST(test_IniReader_MultipleSections);
    RUN_TEST(test_IniReader_IgnoresComments);
    RUN_TEST(test_IniReader_IgnoresEmptyLines);
    RUN_TEST(test_IniReader_HandlesWhitespace);

    return UNITY_END();
}
