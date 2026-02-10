#include <unity.h>

#include <FilterFactory.h>

#include <string>

void setUp(void)
{
}

void tearDown(void)
{
}

void FilterFactory_GetKnownFiltersIni()
{
    // Arrange
    std::string expectedIniContent =
        "[Filter]\n"
        "Type=Switch\n"
        "Input.0=Toggle,DigitalValue\n"
        "Input.1=Feedback,DigitalValue\n"
        "Output.0=Control,DigitalValue\n"
        "Output.1=Feedback,DigitalValue\n"
        "\n"
        "[Filter]\n"
        "Type=Light\n"
        "Input.0=Toggle,DigitalValue\n"
        "Input.1=Feedback,DigitalValue\n"
        "Output.0=Control,DigitalValue\n"
        "Output.1=Feedback,DigitalValue\n"
        "\n"
        "[Filter]\n"
        "Type=Dimmer\n"
        "Input.0=Toggle,DigitalValue\n"
        "Output.0=Control,DimmerControlValue\n"
        "Output.1=Feedback,DigitalValue\n"
        "\n"
        "[Filter]\n"
        "Type=Shutter\n"
        "Input.0=Open,DigitalValue\n"
        "Input.1=Close,DigitalValue\n"
        "Input.2=Stop,DigitalValue\n"
        "Input.3=OpenFeedback,DigitalValue\n"
        "Input.4=CloseFeedback,DigitalValue\n"
        "Output.0=Open,DigitalValue\n"
        "Output.1=Close,DigitalValue\n"
        "\n"
        "[Filter]\n"
        "Type=Climate\n"
        "\n"
        "[Filter]\n"
        "Type=DigitalPassthrough\n"
        "Input.0=Input,DigitalValue\n"
        "Output.0=Output,DigitalValue\n";

    // Act
    std::string iniContent = FilterFactory::GetKnownFiltersIni();

    // Assert
    TEST_ASSERT_EQUAL_STRING(expectedIniContent.c_str(), iniContent.c_str());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(FilterFactory_GetKnownFiltersIni);
    
    return UNITY_END();
}
