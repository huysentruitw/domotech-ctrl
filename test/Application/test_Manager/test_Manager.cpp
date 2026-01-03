#include <unity.h>

#include <Manager.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void Manager_GetKnownFiltersIni()
{
    // Arrange
    Manager manager;
    std::string expectedIniContent =
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
        "Input.2=OpenFeedback,DigitalValue\n"
        "Input.3=CloseFeedback,DigitalValue\n"
        "Output.0=Open,DigitalValue\n"
        "Output.1=Close,DigitalValue\n"
        "\n"
        "[Filter]\n"
        "Type=Toggle\n"
        "Input.0=Toggle,DigitalValue\n"
        "Output.0=Control,DigitalValue\n"
        "Output.1=Feedback,DigitalValue\n";

    // Act
    std::string iniContent = manager.GetKnownFiltersIni();

    // Assert
    TEST_ASSERT_EQUAL_STRING(expectedIniContent.c_str(), iniContent.c_str());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(Manager_GetKnownFiltersIni);
    
    return UNITY_END();
}
