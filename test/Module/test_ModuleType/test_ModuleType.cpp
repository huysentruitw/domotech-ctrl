#include <unity.h>

#include <ModuleType.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void ModuleType_GetModuleTypeNames()
{
    // Arrange
    std::string expectedModuleTypeNames =
        "Unknown\n"
        "Unknown\n"
        "Unknown\n"
        "Dimmer\n"
        "Temperature\n"
        "Unknown\n"
        "PushButtonWithIr\n"
        "PushButton\n"
        "Teleruptor\n"
        "Audio\n"
        "Relais\n"
        "PushButtonWithTemperature\n"
        "PushButtonWithLed\n"
        "Input\n"
        "Unknown\n"
        "Unknown\n";

    // Act
    std::string moduleTypeNames;
    for (auto i = 0; i < 16; i++)
    {
        moduleTypeNames.append(GetModuleTypeName((ModuleType)i));
        moduleTypeNames.append("\n");
    }

    // Assert
    TEST_ASSERT_EQUAL_STRING(expectedModuleTypeNames.c_str(), moduleTypeNames.c_str());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(ModuleType_GetModuleTypeNames);
    
    return UNITY_END();
}
