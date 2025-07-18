#include <unity.h>
#include <memory>
#include <vector>

#include <ModuleScanner.h>
#include <ModuleType.h>
#include <Modules/PushButtonModule.h>
#include <Modules/TeleruptorModule.h>
#include "../../Mocks/MockBus.h"

void setUp(void)
{
    // Setup for each test
}

void tearDown(void)
{
    // Cleanup after each test
}

void ModuleScanner_DetectModules_NoModulesFound()
{
    // Arrange
    MockBus bus;
    ModuleScanner scanner(bus);
    
    // Act
    auto modules = scanner.DetectModules();
    
    // Assert
    TEST_ASSERT_EQUAL(0, modules.size());
    
    // Verify bus was called for addresses 1-127
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(127, bus.LastExchangeAddress); // Last address checked
    TEST_ASSERT_EQUAL(0, bus.LastExchangeData);     // Command 0 used
}

void ModuleScanner_DetectModules_OneModuleFound()
{
    // Arrange
    MockBus bus;
    ModuleScanner scanner(bus);
    
    // Setup mock response for one module at address 42
    ScanResponse response = {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .ModuleType = static_cast<uint8_t>(ModuleType::PushButtons),
        .Data = 0x2000  // 2 buttons
    };
    
    // Queue 41 failed responses (for addresses 1-41)
    for (int i = 0; i < 41; i++) {
        bus.QueueResponse({.Success = false});
    }
    
    // Queue successful response for address 42
    bus.QueueResponse(response);
    
    // Queue remaining failed responses (for addresses 43-127)
    for (int i = 0; i < 85; i++) {
        bus.QueueResponse({.Success = false});
    }
    
    // Act
    auto modules = scanner.DetectModules();
    
    // Assert
    TEST_ASSERT_EQUAL(1, modules.size());
    TEST_ASSERT_NOT_NULL(modules[0].get());
    
    // Verify the module is of the correct type
    auto* pushButtonModule = dynamic_cast<PushButtonModule*>(modules[0].get());
    TEST_ASSERT_NOT_NULL(pushButtonModule);
    
    // Verify bus interactions
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(127, bus.LastExchangeAddress); // Last address checked
}

void ModuleScanner_DetectModules_MultipleModulesFound()
{
    // Arrange
    MockBus bus;
    ModuleScanner scanner(bus);
    
    // Setup mock responses for modules
    ScanResponse pushButtonResponse = {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .ModuleType = static_cast<uint8_t>(ModuleType::PushButtons),
        .Data = 0x4000  // 4 buttons
    };
    
    ScanResponse teleruptorResponse = {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .ModuleType = static_cast<uint8_t>(ModuleType::Teleruptor),
        .Data = 0x0800  // 8 teleruptors
    };
    
    // Queue responses for address 1-9 (failures)
    for (int i = 0; i < 9; i++) {
        bus.QueueResponse({.Success = false});
    }
    
    // Queue response for address 10 (push button)
    bus.QueueResponse(pushButtonResponse);
    
    // Queue responses for address 11-19 (failures)
    for (int i = 0; i < 9; i++) {
        bus.QueueResponse({.Success = false});
    }
    
    // Queue response for address 20 (teleruptor)
    bus.QueueResponse(teleruptorResponse);
    
    // Queue remaining failed responses
    for (int i = 0; i < 107; i++) {
        bus.QueueResponse({.Success = false});
    }
    
    // Act
    auto modules = scanner.DetectModules();
    
    // Assert
    TEST_ASSERT_EQUAL(2, modules.size());
    
    // Verify the first module is a push button module
    auto* pushButtonModule = dynamic_cast<PushButtonModule*>(modules[0].get());
    TEST_ASSERT_NOT_NULL(pushButtonModule);
    
    // Verify the second module is a teleruptor module
    auto* teleruptorModule = dynamic_cast<TeleruptorModule*>(modules[1].get());
    TEST_ASSERT_NOT_NULL(teleruptorModule);
    
    // Verify bus interactions
    TEST_ASSERT_TRUE(bus.ExchangeCalled);
    TEST_ASSERT_EQUAL(127, bus.LastExchangeAddress); // Last address checked
}

void ModuleScanner_DetectModules_UnknownModuleTypeSkipped()
{
    // Arrange
    MockBus bus;
    ModuleScanner scanner(bus);
    
    // Setup mock response for an unknown module type
    ScanResponse unknownResponse = {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .ModuleType = 99,  // Unknown module type
        .Data = 0x1000
    };
    
    // Queue response for address 1 (unknown module type)
    bus.QueueResponse(unknownResponse);
    
    // Queue response for address 2 (valid push button module)
    ScanResponse validResponse = {
        .Success = true,
        .RespondedWithTypeAndData = true,
        .ModuleType = static_cast<uint8_t>(ModuleType::PushButtons),
        .Data = 0x2000
    };
    bus.QueueResponse(validResponse);
    
    // Queue remaining failed responses
    for (int i = 0; i < 125; i++) {
        bus.QueueResponse({.Success = false});
    }
    
    // Act
    auto modules = scanner.DetectModules();
    
    // Assert
    TEST_ASSERT_EQUAL(1, modules.size());  // Only one valid module should be found
    
    // Verify the module is of the correct type
    auto* pushButtonModule = dynamic_cast<PushButtonModule*>(modules[0].get());
    TEST_ASSERT_NOT_NULL(pushButtonModule);
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(ModuleScanner_DetectModules_NoModulesFound);
    RUN_TEST(ModuleScanner_DetectModules_OneModuleFound);
    RUN_TEST(ModuleScanner_DetectModules_MultipleModulesFound);
    RUN_TEST(ModuleScanner_DetectModules_UnknownModuleTypeSkipped);
    
    return UNITY_END();
}
