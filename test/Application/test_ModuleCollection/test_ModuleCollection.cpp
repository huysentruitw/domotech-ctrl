#include <unity.h>
#include <memory>
#include <vector>

#include <ModuleCollection.h>
#include <Module.h>
#include "../../Mocks/MockBus.h"
#include "../../Mocks/MockModule.h"
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

void ModuleCollection_Constructor_FileMissing_InitializesEmpty()
{
    // Arrange & Act
    ModuleCollection collection(mockStorage, "test.ini");

    // Assert
    TEST_ASSERT_EQUAL(0, collection.Count());
}

void ModuleCollection_Constructor_FileWithContent_LoadsModules()
{
    // Arrange
    mockStorage.Format();
    mockStorage.AddTestFile("modules.ini", "[Module]\nType=PushButton\nAddress=42\nInitialData=0000\n");
    ModuleCollection collection(mockStorage, "modules.ini");
    
    // Create a simple factory
    auto factory = [](ModuleType type, uint8_t address, uint16_t initialData)
    {
        return std::make_unique<MockModule>(MockBus(), type, address, initialData);
    };

    // Act
    bool result = collection.LoadFromFile(factory);

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_GREATER_THAN(0, collection.Count());
}

void ModuleCollection_Emplace_AddsSingleModule()
{
    // Arrange
    MockBus bus;
    ModuleCollection collection(mockStorage, "test.ini");
    std::vector<std::unique_ptr<Module>> modules;
    modules.push_back(std::make_unique<MockModule>(bus, ModuleType::PushButton, 1, 0x0000));

    // Act
    bool result = collection.Emplace(std::move(modules));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, collection.Count());
}

void ModuleCollection_Emplace_AddsMultipleModules()
{
    // Arrange
    MockBus bus;
    ModuleCollection collection(mockStorage, "test.ini");
    std::vector<std::unique_ptr<Module>> modules;
    modules.push_back(std::make_unique<MockModule>(bus, ModuleType::PushButton, 1, 0x0000));
    modules.push_back(std::make_unique<MockModule>(bus, ModuleType::Input, 2, 0x0000));
    modules.push_back(std::make_unique<MockModule>(bus, ModuleType::Dimmer, 3, 0x0000));

    // Act
    bool result = collection.Emplace(std::move(modules));

    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(3, collection.Count());
}

void ModuleCollection_TryGetModuleByAddress_ReturnsModuleIfExists()
{
    // Arrange
    MockBus bus;
    ModuleCollection collection(mockStorage, "test.ini");
    std::vector<std::unique_ptr<Module>> modules;
    modules.push_back(std::make_unique<MockModule>(bus, ModuleType::PushButton, 42, 0x0000));
    collection.Emplace(std::move(modules));

    // Act
    auto module = collection.TryGetModuleByAddress(42);

    // Assert
    TEST_ASSERT_NOT_NULL(module.get());
    TEST_ASSERT_EQUAL(42, module->GetAddress());
    TEST_ASSERT_EQUAL(ModuleType::PushButton, module->GetType());
}

void ModuleCollection_TryGetModuleByAddress_ReturnsNullIfNotExists()
{
    // Arrange
    MockBus bus;
    ModuleCollection collection(mockStorage, "test.ini");
    std::vector<std::unique_ptr<Module>> modules;
    modules.push_back(std::make_unique<MockModule>(bus, ModuleType::PushButton, 42, 0x0000));
    collection.Emplace(std::move(modules));

    // Act
    auto module = collection.TryGetModuleByAddress(99);

    // Assert
    TEST_ASSERT_NULL(module.get());
}

void ModuleCollection_Iterator_IteratesAllModules()
{
    // Arrange
    MockBus bus;
    ModuleCollection collection(mockStorage, "test.ini");
    std::vector<std::unique_ptr<Module>> modules;
    modules.push_back(std::make_unique<MockModule>(bus, ModuleType::PushButton, 1, 0x0000));
    modules.push_back(std::make_unique<MockModule>(bus, ModuleType::Input, 2, 0x0000));
    modules.push_back(std::make_unique<MockModule>(bus, ModuleType::Dimmer, 3, 0x0000));
    collection.Emplace(std::move(modules));

    // Act & Assert
    int count = 0;
    for (const auto& module : collection)
    {
        count++;
        TEST_ASSERT_NOT_NULL(module.get());
    }
    
    TEST_ASSERT_EQUAL(3, count);
}

void ModuleCollection_Emplace_ReplacesExistingModules()
{
    // Arrange
    MockBus bus;
    ModuleCollection collection(mockStorage, "test.ini");
    
    // Add first set of modules
    std::vector<std::unique_ptr<Module>> modules1;
    modules1.push_back(std::make_unique<MockModule>(bus, ModuleType::PushButton, 1, 0x0000));
    collection.Emplace(std::move(modules1));
    TEST_ASSERT_EQUAL(1, collection.Count());

    // Add second set of modules
    std::vector<std::unique_ptr<Module>> modules2;
    modules2.push_back(std::make_unique<MockModule>(bus, ModuleType::Input, 2, 0x0000));
    modules2.push_back(std::make_unique<MockModule>(bus, ModuleType::Dimmer, 3, 0x0000));
    
    // Act
    collection.Emplace(std::move(modules2));

    // Assert - old module should be gone, new modules should be there
    TEST_ASSERT_EQUAL(2, collection.Count());
    TEST_ASSERT_NULL(collection.TryGetModuleByAddress(1).get());
    TEST_ASSERT_NOT_NULL(collection.TryGetModuleByAddress(2).get());
    TEST_ASSERT_NOT_NULL(collection.TryGetModuleByAddress(3).get());
}

int main()
{
    UNITY_BEGIN();
    
    RUN_TEST(ModuleCollection_Constructor_FileMissing_InitializesEmpty);
    RUN_TEST(ModuleCollection_Constructor_FileWithContent_LoadsModules);
    RUN_TEST(ModuleCollection_Emplace_AddsSingleModule);
    RUN_TEST(ModuleCollection_Emplace_AddsMultipleModules);
    RUN_TEST(ModuleCollection_TryGetModuleByAddress_ReturnsModuleIfExists);
    RUN_TEST(ModuleCollection_TryGetModuleByAddress_ReturnsNullIfNotExists);
    RUN_TEST(ModuleCollection_Iterator_IteratesAllModules);
    RUN_TEST(ModuleCollection_Emplace_ReplacesExistingModules);

    return UNITY_END();
}
