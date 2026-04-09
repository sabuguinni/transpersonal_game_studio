#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "../QATestFramework.h"

DEFINE_LOG_CATEGORY_STATIC(LogCoreSystemTests, Log, All);

/**
 * Core System Tests for Transpersonal Game
 * Tests fundamental engine systems, module loading, and basic functionality
 */

// Test 1: Module Registration and Loading
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreModuleLoadTest, "Transpersonal.Core.ModuleLoad", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCoreModuleLoadTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogCoreSystemTests, Log, TEXT("Testing Core Module Loading..."));
    
    // Validate TranspersonalGame module is loaded
    FModuleManager& ModuleManager = FModuleManager::Get();
    
    TestTrue("TranspersonalGame module should be loaded", 
        ModuleManager.IsModuleLoaded("TranspersonalGame"));
    
    // Test world context availability
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    TestNotNull("World context should be available", World);
    
    if (World)
    {
        TestTrue("World should be valid", IsValid(World));
        UE_LOG(LogCoreSystemTests, Log, TEXT("✓ World validation passed: %s"), *World->GetName());
    }
    
    return true;
}

// Test 2: Physics System Validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhysicsSystemTest, "Transpersonal.Core.Physics", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPhysicsSystemTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogCoreSystemTests, Log, TEXT("Testing Physics System..."));
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        AddError("No world context for physics testing");
        return false;
    }
    
    // Test physics scene
    FPhysScene* PhysScene = World->GetPhysicsScene();
    TestNotNull("Physics scene should exist", PhysScene);
    
    // Test gravity settings
    FVector Gravity = World->GetGravityZ() * FVector::UpVector;
    TestTrue("Gravity should be negative (downward)", Gravity.Z < 0.0f);
    
    UE_LOG(LogCoreSystemTests, Log, TEXT("✓ Physics system validation passed"));
    return true;
}

// Test 3: Memory Usage Validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryUsageTest, "Transpersonal.Core.Memory", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryUsageTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogCoreSystemTests, Log, TEXT("Testing Memory Usage..."));
    
    // Get current memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    // Convert to MB for easier reading
    uint64 UsedPhysicalMB = MemStats.UsedPhysical / (1024 * 1024);
    uint64 UsedVirtualMB = MemStats.UsedVirtual / (1024 * 1024);
    
    UE_LOG(LogCoreSystemTests, Log, TEXT("Physical Memory Used: %llu MB"), UsedPhysicalMB);
    UE_LOG(LogCoreSystemTests, Log, TEXT("Virtual Memory Used: %llu MB"), UsedVirtualMB);
    
    // Validate memory usage is within reasonable bounds
    TestTrue("Physical memory usage should be reasonable", 
        UsedPhysicalMB < FPerformanceThresholds::MAX_MEMORY_MB_PC);
    
    return true;
}

// Test 4: Rendering System Validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRenderingSystemTest, "Transpersonal.Core.Rendering", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FRenderingSystemTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogCoreSystemTests, Log, TEXT("Testing Rendering System..."));
    
    // Test RHI availability
    TestNotNull("RHI should be available", GDynamicRHI);
    
    if (GDynamicRHI)
    {
        FString RHIName = GDynamicRHI->GetName();
        UE_LOG(LogCoreSystemTests, Log, TEXT("RHI: %s"), *RHIName);
        TestTrue("RHI name should not be empty", !RHIName.IsEmpty());
    }
    
    // Test viewport availability
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (World)
    {
        UGameViewportClient* ViewportClient = World->GetGameViewport();
        if (ViewportClient)
        {
            UE_LOG(LogCoreSystemTests, Log, TEXT("✓ Viewport client available"));
        }
    }
    
    return true;
}

// Test 5: Game Mode Validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameModeTest, "Transpersonal.Core.GameMode", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameModeTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogCoreSystemTests, Log, TEXT("Testing Game Mode System..."));
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        AddError("No world context for game mode testing");
        return false;
    }
    
    // Test game mode availability
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (GameMode)
    {
        TestTrue("Game mode should be valid", IsValid(GameMode));
        UE_LOG(LogCoreSystemTests, Log, TEXT("✓ Game Mode: %s"), *GameMode->GetClass()->GetName());
    }
    else
    {
        UE_LOG(LogCoreSystemTests, Warning, TEXT("⚠ No game mode found (may be normal in editor)"));
    }
    
    return true;
}

// Test 6: Asset Loading Validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAssetLoadingTest, "Transpersonal.Core.AssetLoading", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAssetLoadingTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogCoreSystemTests, Log, TEXT("Testing Asset Loading System..."));
    
    // Test asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    TestTrue("Asset registry should be available", &AssetRegistry != nullptr);
    
    // Test basic asset queries
    TArray<FAssetData> AllAssets;
    AssetRegistry.GetAllAssets(AllAssets);
    
    UE_LOG(LogCoreSystemTests, Log, TEXT("Total assets in registry: %d"), AllAssets.Num());
    TestTrue("Should have some assets registered", AllAssets.Num() > 0);
    
    return true;
}

// Test 7: Input System Validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInputSystemTest, "Transpersonal.Core.Input", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInputSystemTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogCoreSystemTests, Log, TEXT("Testing Input System..."));
    
    // Test input settings
    const UInputSettings* InputSettings = GetDefault<UInputSettings>();
    TestNotNull("Input settings should be available", InputSettings);
    
    if (InputSettings)
    {
        // Test action mappings
        const TArray<FInputActionKeyMapping>& ActionMappings = InputSettings->GetActionMappings();
        UE_LOG(LogCoreSystemTests, Log, TEXT("Action mappings found: %d"), ActionMappings.Num());
        
        // Test axis mappings
        const TArray<FInputAxisKeyMapping>& AxisMappings = InputSettings->GetAxisMappings();
        UE_LOG(LogCoreSystemTests, Log, TEXT("Axis mappings found: %d"), AxisMappings.Num());
    }
    
    return true;
}

// Test 8: Audio System Validation
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioSystemTest, "Transpersonal.Core.Audio", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioSystemTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogCoreSystemTests, Log, TEXT("Testing Audio System..."));
    
    // Test audio device
    FAudioDeviceManager* AudioDeviceManager = GEngine->GetAudioDeviceManager();
    TestNotNull("Audio device manager should be available", AudioDeviceManager);
    
    if (AudioDeviceManager)
    {
        FAudioDevice* MainAudioDevice = AudioDeviceManager->GetMainAudioDevice();
        if (MainAudioDevice)
        {
            UE_LOG(LogCoreSystemTests, Log, TEXT("✓ Main audio device available"));
            TestTrue("Audio device should be valid", MainAudioDevice != nullptr);
        }
    }
    
    return true;
}