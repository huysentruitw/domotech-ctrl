#include <unity.h>

#include <FilterType.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void FilterType_GetFilterTypeNames()
{
    // Arrange
    std::string expectedFilterTypeNames =
        "Unknown\n"
        "Toggle\n"
        "Shutter\n"
        "Dimmer\n"
        "Unknown\n"
        "Unknown\n"
        "Unknown\n"
        "Unknown\n";

    // Act
    std::string filterTypeNames;
    for (auto i = 0; i < 8; i++) {
        filterTypeNames.append(GetFilterTypeName((FilterType)i));
        filterTypeNames.append("\n");
    }

    // Assert
    TEST_ASSERT_EQUAL_STRING(expectedFilterTypeNames.c_str(), filterTypeNames.c_str());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(FilterType_GetFilterTypeNames);
    
    return UNITY_END();
}
