#include <memory>

#include <unity.h>

#include <Configuration.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void Configuration_GetKnownFiltersIni()
{
    // Arrange
    Configuration config;
    std::string expectedIniContent=
        "[Filter:DimmerFilter]\n"
        "Input:0=Toggle,DigitalValue\n"
        "Output:0=Control,DimmerControlValue\n"
        "\n"
        "[Filter:ShutterFilter]\n"
        "Input:0=Open,DigitalValue\n"
        "Input:1=Close,DigitalValue\n"
        "Input:2=OpenFeedback,DigitalValue\n"
        "Input:3=CloseFeedback,DigitalValue\n"
        "Output:0=Open,DigitalValue\n"
        "Output:1=Close,DigitalValue\n"
        "\n"
        "[Filter:ToggleFilter]\n"
        "Input:0=Toggle,DigitalValue\n"
        "Output:0=Control,DigitalValue\n";

    // Act
    std::string iniContent=config.GetKnownFiltersIni();

    // Assert
    TEST_ASSERT_EQUAL_STRING(expectedIniContent.c_str(), iniContent.c_str());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(Configuration_GetKnownFiltersIni);
    
    return UNITY_END();
}
