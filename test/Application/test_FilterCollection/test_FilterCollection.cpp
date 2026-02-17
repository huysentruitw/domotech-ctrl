#include <unity.h>
#include <memory>
#include <vector>

#include <FilterCollection.h>
#include <Filter.h>
#include "../../Mocks/MockFilter.h"
#include "../../Mocks/MockStorage.h"

static MockStorage mockStorage;

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void FilterCollection_Constructor_FileMissing_InitializesEmpty()
{
    // Arrange & Act
    FilterCollection collection(mockStorage, "filters.ini");

    // Assert
    int count = 0;
    for (const auto& filter : collection)
    {
        (void)filter;
        count++;
    }

    TEST_ASSERT_EQUAL(0, count);
}

static void DebugTypes() { printf("sizeof(Filter) = %zu, alignof(Filter) = %zu\n", sizeof(Filter), alignof(Filter)); printf("sizeof(MockFilter) = %zu, alignof(MockFilter) = %zu\n", sizeof(MockFilter), alignof(MockFilter)); }

void FilterCollection_Constructor_FileWithContent_LoadsFilters()
{
    DebugTypes();

    // Arrange
    mockStorage.Format();
    mockStorage.AddTestFile("filters.ini", "[Filter]\nId=TestFilter\nType=Switch\nConnections=ABC\n");
    FilterCollection collection(mockStorage, "filters.ini");
    
    // Create a simple factory that properly converts string_views to strings
    auto factory = [](std::string_view id, std::string_view typeName, std::string_view connections)
    {
        return std::make_unique<MockFilter>(id);
    };

    // Act
    bool result = collection.LoadFromFile(factory);

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_NOT_NULL(collection.TryGetFilterById("TestFilter"));
}

void FilterCollection_AppendFilter_AddsSingleFilter()
{
    // Arrange
    FilterCollection collection(mockStorage, "filters.ini");
    auto filter = std::make_unique<MockFilter>("filter1");

    // Act
    auto result = collection.AppendFilter(std::move(filter), "connections");

    // Assert
    TEST_ASSERT_NOT_NULL(result.get());
    TEST_ASSERT_EQUAL_STRING("filter1", result->GetId().data());
}

void FilterCollection_AppendFilter_AddsMultipleFilters()
{
    // Arrange
    FilterCollection collection(mockStorage, "filters.ini");

    // Act
    auto filter1 = collection.AppendFilter(std::make_unique<MockFilter>("filter1"), "conn1");
    auto filter2 = collection.AppendFilter(std::make_unique<MockFilter>("filter2"), "conn2");
    auto filter3 = collection.AppendFilter(std::make_unique<MockFilter>("filter3"), "conn3");

    // Assert
    TEST_ASSERT_NOT_NULL(filter1.get());
    TEST_ASSERT_NOT_NULL(filter2.get());
    TEST_ASSERT_NOT_NULL(filter3.get());
}

void FilterCollection_TryGetFilterById_ReturnsFilterIfExists()
{
    // Arrange
    FilterCollection collection(mockStorage, "filters.ini");
    collection.AppendFilter(std::make_unique<MockFilter>("test_filter"), "connections");

    // Act
    auto filter = collection.TryGetFilterById("test_filter");

    // Assert
    TEST_ASSERT_NOT_NULL(filter.get());
    TEST_ASSERT_EQUAL_STRING("test_filter", filter->GetId().data());
    TEST_ASSERT_EQUAL(FilterType::Switch, filter->GetType());
}

void FilterCollection_TryGetFilterById_ReturnsNullIfNotExists()
{
    // Arrange
    FilterCollection collection(mockStorage, "filters.ini");
    collection.AppendFilter(std::make_unique<MockFilter>("filter1"), "connections");

    // Act
    auto filter = collection.TryGetFilterById("nonexistent");

    // Assert
    TEST_ASSERT_NULL(filter.get());
}

void FilterCollection_Clear_RemovesAllFilters()
{
    // Arrange
    FilterCollection collection(mockStorage, "filters.ini");
    collection.AppendFilter(std::make_unique<MockFilter>("filter1"), "conn1");
    collection.AppendFilter(std::make_unique<MockFilter>("filter2"), "conn2");

    // Act
    bool result = collection.Clear();

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_NULL(collection.TryGetFilterById("filter1").get());
    TEST_ASSERT_NULL(collection.TryGetFilterById("filter2").get());
}

void FilterCollection_Iterator_IteratesAllFilters()
{
    // Arrange
    FilterCollection collection(mockStorage, "filters.ini");
    collection.AppendFilter(std::make_unique<MockFilter>("filter1"), "conn1");
    collection.AppendFilter(std::make_unique<MockFilter>("filter2"), "conn2");
    collection.AppendFilter(std::make_unique<MockFilter>("filter3"), "conn3");

    // Act & Assert
    int count = 0;
    for (const auto& filter : collection)
    {
        count++;
        TEST_ASSERT_NOT_NULL(filter.second.get());
    }

    TEST_ASSERT_EQUAL(3, count);
}

void FilterCollection_AppendFilter_ReturnsSharedPointer()
{
    // Arrange
    FilterCollection collection(mockStorage, "filters.ini");
    auto filter = std::make_unique<MockFilter>("filter1");

    // Act
    auto sharedPtr = collection.AppendFilter(std::move(filter), "connections");

    // Assert
    TEST_ASSERT_NOT_NULL(sharedPtr.get());
    // Verify same instance can be retrieved
    auto retrieved = collection.TryGetFilterById("filter1");
    TEST_ASSERT_EQUAL_PTR(sharedPtr.get(), retrieved.get());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(FilterCollection_Constructor_FileMissing_InitializesEmpty);
    RUN_TEST(FilterCollection_Constructor_FileWithContent_LoadsFilters);
    RUN_TEST(FilterCollection_AppendFilter_AddsSingleFilter);
    RUN_TEST(FilterCollection_AppendFilter_AddsMultipleFilters);
    RUN_TEST(FilterCollection_TryGetFilterById_ReturnsFilterIfExists);
    RUN_TEST(FilterCollection_TryGetFilterById_ReturnsNullIfNotExists);
    RUN_TEST(FilterCollection_Clear_RemovesAllFilters);
    RUN_TEST(FilterCollection_Iterator_IteratesAllFilters);
    RUN_TEST(FilterCollection_AppendFilter_ReturnsSharedPointer);

    return UNITY_END();
}
