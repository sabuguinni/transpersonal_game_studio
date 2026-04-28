#pragma once

#include "CoreMinimal.h"
#include "../QATestFramework.h"
#include "Misc/AutomationTest.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIntegrationTests, Log, All);

/**
 * Base class for integration tests that validate multiple systems working together
 */
class TRANSPERSONALGAME_API FIntegrationTestBase : public FAutomationTestBase
{
public:
    FIntegrationTestBase(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

protected:
    // Game flow testing
    bool TestCompleteGameFlow();
    bool TestSystemsIntegration();
    bool TestFullLoadPerformance();
    
    // Cross-system integration
    bool TestAIPhysicsIntegration();
    bool TestSurvivalCombatIntegration();
    bool TestWorldPerformanceIntegration();
    
    // Save/Load system testing
    bool TestGameStateSaving();
    bool TestGameStateLoading();
    bool TestSaveFileIntegrity();
    
    // Multiplayer integration
    bool TestNetworkReplication();
    bool TestClientServerSync();
    bool TestMultiplayerPerformance();
    
    // Platform compatibility
    bool TestPCCompatibility();
    bool TestConsoleCompatibility();
    bool TestInputCompatibility();
    
    // Memory management
    bool TestMemoryAllocation();
    bool TestGarbageCollectionEfficiency();
    bool TestMemoryLeakDetection();
    
    // Utility methods
    UWorld* GetTestWorld();
};

/**
 * Full Game Integration Test
 * Tests complete game flow from start to finish
 */
class TRANSPERSONALGAME_API FFullGameIntegrationTest : public FIntegrationTestBase
{
public:
    FFullGameIntegrationTest()
        : FIntegrationTestBase(TEXT("Transpersonal.Integration.FullGame"), true)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Cross-System Integration Test
 * Tests multiple systems working together
 */
class TRANSPERSONALGAME_API FCrossSystemIntegrationTest : public FIntegrationTestBase
{
public:
    FCrossSystemIntegrationTest()
        : FIntegrationTestBase(TEXT("Transpersonal.Integration.CrossSystem"), false)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Save/Load System Test
 * Tests game state persistence and restoration
 */
class TRANSPERSONALGAME_API FSaveLoadSystemTest : public FIntegrationTestBase
{
public:
    FSaveLoadSystemTest()
        : FIntegrationTestBase(TEXT("Transpersonal.Integration.SaveLoad"), false)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Multiplayer Integration Test
 * Tests network functionality and multiplayer systems
 */
class TRANSPERSONALGAME_API FMultiplayerIntegrationTest : public FIntegrationTestBase
{
public:
    FMultiplayerIntegrationTest()
        : FIntegrationTestBase(TEXT("Transpersonal.Integration.Multiplayer"), true)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Platform Compatibility Test
 * Tests compatibility across different platforms and configurations
 */
class TRANSPERSONALGAME_API FPlatformCompatibilityTest : public FIntegrationTestBase
{
public:
    FPlatformCompatibilityTest()
        : FIntegrationTestBase(TEXT("Transpersonal.Integration.PlatformCompatibility"), true)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Memory Management Integration Test
 * Tests memory allocation, garbage collection, and leak detection
 */
class TRANSPERSONALGAME_API FMemoryManagementTest : public FIntegrationTestBase
{
public:
    FMemoryManagementTest()
        : FIntegrationTestBase(TEXT("Transpersonal.Integration.MemoryManagement"), true)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};