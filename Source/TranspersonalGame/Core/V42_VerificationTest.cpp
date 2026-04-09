// Copyright Transpersonal Game Studio. All Rights Reserved.
// V42_VerificationTest.cpp - Comprehensive verification test for v4.2 deployment

#include "V42_VerificationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "StudioDirector.h"
#include "TranspersonalCoreSubsystem.h"
#include "EngineArchitecture.h"

UV42VerificationTest::UV42VerificationTest()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    TestResults.Empty();
    bTestsCompleted = false;
    TestStartTime = 0.0f;
}

void UV42VerificationTest::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("=== TRANSPERSONAL GAME STUDIO v4.2 VERIFICATION TEST STARTING ==="));
    
    TestStartTime = GetWorld()->GetTimeSeconds();
    
    // Schedule tests to run after a brief delay to ensure all systems are initialized
    GetWorld()->GetTimerManager().SetTimer(
        TestTimerHandle,
        this,
        &UV42VerificationTest::RunVerificationTests,
        1.0f,
        false
    );
}

void UV42VerificationTest::RunVerificationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting comprehensive v4.2 verification tests..."));
    
    // Test 1: Core Subsystem Verification
    TestCoreSubsystems();
    
    // Test 2: Studio Director System
    TestStudioDirectorSystem();
    
    // Test 3: Engine Architecture
    TestEngineArchitecture();
    
    // Test 4: Module Registration
    TestModuleRegistration();
    
    // Test 5: Performance Baseline
    TestPerformanceBaseline();
    
    // Test 6: Asset System
    TestAssetSystem();
    
    // Test 7: Memory Management
    TestMemoryManagement();
    
    // Test 8: Threading Safety
    TestThreadingSafety();
    
    // Generate final report
    GenerateVerificationReport();
    
    bTestsCompleted = true;
    
    UE_LOG(LogTemp, Warning, TEXT("=== VERIFICATION TESTS COMPLETED ==="));
}

void UV42VerificationTest::TestCoreSubsystems()
{
    FVerificationTestResult Result;
    Result.TestName = TEXT("Core Subsystems");
    Result.bPassed = true;
    Result.Details.Empty();
    
    // Test TranspersonalCoreSubsystem
    UTranspersonalCoreSubsystem* CoreSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UTranspersonalCoreSubsystem>();
    if (CoreSubsystem)
    {
        Result.Details.Add(TEXT("✓ TranspersonalCoreSubsystem accessible"));
        if (CoreSubsystem->IsInitialized())
        {
            Result.Details.Add(TEXT("✓ TranspersonalCoreSubsystem initialized"));
        }
        else
        {
            Result.Details.Add(TEXT("⚠ TranspersonalCoreSubsystem not initialized"));
            Result.bPassed = false;
        }
    }
    else
    {
        Result.Details.Add(TEXT("✗ TranspersonalCoreSubsystem not found"));
        Result.bPassed = false;
    }
    
    // Test Studio Director Subsystem
    UStudioDirectorSubsystem* DirectorSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStudioDirectorSubsystem>();
    if (DirectorSubsystem)
    {
        Result.Details.Add(TEXT("✓ StudioDirectorSubsystem accessible"));
        
        TArray<FSystemInfo> RegisteredSystems = DirectorSubsystem->GetAllSystems();
        Result.Details.Add(FString::Printf(TEXT("✓ %d systems registered with Studio Director"), RegisteredSystems.Num()));
    }
    else
    {
        Result.Details.Add(TEXT("⚠ StudioDirectorSubsystem not found"));
    }
    
    TestResults.Add(Result);
}

void UV42VerificationTest::TestStudioDirectorSystem()
{
    FVerificationTestResult Result;
    Result.TestName = TEXT("Studio Director System");
    Result.bPassed = true;
    Result.Details.Empty();
    
    UStudioDirectorSubsystem* DirectorSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStudioDirectorSubsystem>();
    if (DirectorSubsystem)
    {
        // Test system registration
        FSystemInfo TestSystemInfo;
        TestSystemInfo.SystemName = TEXT("VerificationTestSystem");
        TestSystemInfo.AgentName = TEXT("Test Agent");
        TestSystemInfo.AgentNumber = 99;
        TestSystemInfo.Priority = EStudioDirectorPriority::Low;
        TestSystemInfo.Status = ESystemStatus::Online;
        
        DirectorSubsystem->RegisterSystem(TestSystemInfo);
        
        ESystemStatus RetrievedStatus = DirectorSubsystem->GetSystemStatus(TEXT("VerificationTestSystem"));
        if (RetrievedStatus == ESystemStatus::Online)
        {
            Result.Details.Add(TEXT("✓ System registration and retrieval working"));
        }
        else
        {
            Result.Details.Add(TEXT("✗ System registration failed"));
            Result.bPassed = false;
        }
        
        // Test creative vision compliance
        bool bCompliant = DirectorSubsystem->IsCreativeVisionCompliant();
        Result.Details.Add(FString::Printf(TEXT("Creative Vision Compliance: %s"), 
            bCompliant ? TEXT("✓ COMPLIANT") : TEXT("⚠ NON-COMPLIANT")));
        
        // Test performance monitoring
        bool bPerformanceOK = DirectorSubsystem->IsOverallPerformanceAcceptable();
        Result.Details.Add(FString::Printf(TEXT("Performance Status: %s"), 
            bPerformanceOK ? TEXT("✓ ACCEPTABLE") : TEXT("⚠ NEEDS ATTENTION")));
        
        // Clean up test system
        DirectorSubsystem->UnregisterSystem(TEXT("VerificationTestSystem"));
    }
    else
    {
        Result.Details.Add(TEXT("✗ Studio Director Subsystem not available"));
        Result.bPassed = false;
    }
    
    TestResults.Add(Result);
}

void UV42VerificationTest::TestEngineArchitecture()
{
    FVerificationTestResult Result;
    Result.TestName = TEXT("Engine Architecture");
    Result.bPassed = true;
    Result.Details.Empty();
    
    // Test if we can create engine architecture validator
    UEngineArchitectureValidator* Validator = NewObject<UEngineArchitectureValidator>();
    if (Validator)
    {
        Result.Details.Add(TEXT("✓ EngineArchitectureValidator created"));
        
        // Test validation capabilities
        bool bArchitectureValid = Validator->ValidateArchitecture();
        Result.Details.Add(FString::Printf(TEXT("Architecture Validation: %s"), 
            bArchitectureValid ? TEXT("✓ VALID") : TEXT("⚠ ISSUES DETECTED")));
        
        // Test performance standards
        bool bPerformanceStandards = Validator->ValidatePerformanceStandards();
        Result.Details.Add(FString::Printf(TEXT("Performance Standards: %s"), 
            bPerformanceStandards ? TEXT("✓ MET") : TEXT("⚠ NOT MET")));
        
        if (!bArchitectureValid || !bPerformanceStandards)
        {
            Result.bPassed = false;
        }
    }
    else
    {
        Result.Details.Add(TEXT("✗ Failed to create EngineArchitectureValidator"));
        Result.bPassed = false;
    }
    
    TestResults.Add(Result);
}

void UV42VerificationTest::TestModuleRegistration()
{
    FVerificationTestResult Result;
    Result.TestName = TEXT("Module Registration");
    Result.bPassed = true;
    Result.Details.Empty();
    
    // Check if our game module is properly loaded
    FModuleManager& ModuleManager = FModuleManager::Get();
    
    // Test core module
    if (ModuleManager.IsModuleLoaded(TEXT("TranspersonalGame")))
    {
        Result.Details.Add(TEXT("✓ TranspersonalGame module loaded"));
    }
    else
    {
        Result.Details.Add(TEXT("✗ TranspersonalGame module not loaded"));
        Result.bPassed = false;
    }
    
    // Test core systems module
    if (ModuleManager.IsModuleLoaded(TEXT("TranspersonalGameCore")))
    {
        Result.Details.Add(TEXT("✓ TranspersonalGameCore module loaded"));
    }
    else
    {
        Result.Details.Add(TEXT("⚠ TranspersonalGameCore module not loaded"));
    }
    
    // List all loaded modules for debugging
    TArray<FName> LoadedModules;
    ModuleManager.FindModules(TEXT("Transpersonal*"), LoadedModules);
    
    Result.Details.Add(FString::Printf(TEXT("Total Transpersonal modules loaded: %d"), LoadedModules.Num()));
    for (const FName& ModuleName : LoadedModules)
    {
        Result.Details.Add(FString::Printf(TEXT("  - %s"), *ModuleName.ToString()));
    }
    
    TestResults.Add(Result);
}

void UV42VerificationTest::TestPerformanceBaseline()
{
    FVerificationTestResult Result;
    Result.TestName = TEXT("Performance Baseline");
    Result.bPassed = true;
    Result.Details.Empty();
    
    // Measure current frame time
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds();
    Result.Details.Add(FString::Printf(TEXT("Current Frame Time: %.3fms"), CurrentFrameTime * 1000.0f));
    
    // Check if we're meeting target frame rates
    float TargetFrameTime = 1.0f / 60.0f; // 60 FPS target
    if (CurrentFrameTime <= TargetFrameTime * 1.2f) // Allow 20% tolerance
    {
        Result.Details.Add(TEXT("✓ Frame rate within acceptable range"));
    }
    else
    {
        Result.Details.Add(TEXT("⚠ Frame rate below target"));
        Result.bPassed = false;
    }
    
    // Memory usage check
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    Result.Details.Add(FString::Printf(TEXT("Memory Usage: %.1f MB"), MemoryUsageMB));
    
    // Basic memory threshold check (adjust as needed)
    if (MemoryUsageMB < 2048.0f) // 2GB threshold
    {
        Result.Details.Add(TEXT("✓ Memory usage within acceptable range"));
    }
    else
    {
        Result.Details.Add(TEXT("⚠ High memory usage detected"));
    }
    
    TestResults.Add(Result);
}

void UV42VerificationTest::TestAssetSystem()
{
    FVerificationTestResult Result;
    Result.TestName = TEXT("Asset System");
    Result.bPassed = true;
    Result.Details.Empty();
    
    // Test asset registry access
    IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
    
    if (AssetRegistry.IsLoadingAssets())
    {
        Result.Details.Add(TEXT("⚠ Asset registry still loading"));
    }
    else
    {
        Result.Details.Add(TEXT("✓ Asset registry loaded"));
    }
    
    // Count assets in our project
    TArray<FAssetData> AllAssets;
    AssetRegistry.GetAllAssets(AllAssets);
    
    int32 TranspersonalAssets = 0;
    for (const FAssetData& Asset : AllAssets)
    {
        if (Asset.PackageName.ToString().Contains(TEXT("TranspersonalGame")))
        {
            TranspersonalAssets++;
        }
    }
    
    Result.Details.Add(FString::Printf(TEXT("Total assets in project: %d"), AllAssets.Num()));
    Result.Details.Add(FString::Printf(TEXT("Transpersonal-specific assets: %d"), TranspersonalAssets));
    
    TestResults.Add(Result);
}

void UV42VerificationTest::TestMemoryManagement()
{
    FVerificationTestResult Result;
    Result.TestName = TEXT("Memory Management");
    Result.bPassed = true;
    Result.Details.Empty();
    
    // Test object creation and cleanup
    FPlatformMemoryStats InitialStats = FPlatformMemory::GetStats();
    
    // Create test objects
    TArray<UObject*> TestObjects;
    for (int32 i = 0; i < 1000; ++i)
    {
        UObject* TestObj = NewObject<UObject>();
        TestObjects.Add(TestObj);
    }
    
    FPlatformMemoryStats PostCreationStats = FPlatformMemory::GetStats();
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    FPlatformMemoryStats PostGCStats = FPlatformMemory::GetStats();
    
    float MemoryDelta = (PostGCStats.UsedPhysical - InitialStats.UsedPhysical) / (1024.0f * 1024.0f);
    Result.Details.Add(FString::Printf(TEXT("Memory delta after GC: %.2f MB"), MemoryDelta));
    
    if (MemoryDelta < 10.0f) // Should be minimal after GC
    {
        Result.Details.Add(TEXT("✓ Memory management working correctly"));
    }
    else
    {
        Result.Details.Add(TEXT("⚠ Potential memory leak detected"));
        Result.bPassed = false;
    }
    
    TestResults.Add(Result);
}

void UV42VerificationTest::TestThreadingSafety()
{
    FVerificationTestResult Result;
    Result.TestName = TEXT("Threading Safety");
    Result.bPassed = true;
    Result.Details.Empty();
    
    // Test basic thread safety mechanisms
    FCriticalSection TestCriticalSection;
    std::atomic<int32> TestCounter(0);
    
    // Simple thread safety test
    {
        FScopeLock Lock(&TestCriticalSection);
        TestCounter++;
        Result.Details.Add(TEXT("✓ Critical section locking functional"));
    }
    
    // Test atomic operations
    int32 ExpectedValue = 1;
    if (TestCounter.compare_exchange_strong(ExpectedValue, 2))
    {
        Result.Details.Add(TEXT("✓ Atomic operations functional"));
    }
    else
    {
        Result.Details.Add(TEXT("✗ Atomic operations failed"));
        Result.bPassed = false;
    }
    
    // Check if we're on the game thread
    if (IsInGameThread())
    {
        Result.Details.Add(TEXT("✓ Running on game thread as expected"));
    }
    else
    {
        Result.Details.Add(TEXT("⚠ Not running on game thread"));
    }
    
    TestResults.Add(Result);
}

void UV42VerificationTest::GenerateVerificationReport()
{
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("=== TRANSPERSONAL GAME STUDIO v4.2 VERIFICATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT(""));
    
    float TestDuration = GetWorld()->GetTimeSeconds() - TestStartTime;
    UE_LOG(LogTemp, Warning, TEXT("Test Duration: %.2f seconds"), TestDuration);
    UE_LOG(LogTemp, Warning, TEXT(""));
    
    int32 PassedTests = 0;
    int32 TotalTests = TestResults.Num();
    
    for (const FVerificationTestResult& TestResult : TestResults)
    {
        if (TestResult.bPassed)
        {
            PassedTests++;
            UE_LOG(LogTemp, Warning, TEXT("✓ %s: PASSED"), *TestResult.TestName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("✗ %s: FAILED"), *TestResult.TestName);
        }
        
        for (const FString& Detail : TestResult.Details)
        {
            UE_LOG(LogTemp, Warning, TEXT("    %s"), *Detail);
        }
        UE_LOG(LogTemp, Warning, TEXT(""));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== SUMMARY ==="));
    UE_LOG(LogTemp, Warning, TEXT("Tests Passed: %d/%d"), PassedTests, TotalTests);
    
    float SuccessRate = (float)PassedTests / (float)TotalTests * 100.0f;
    UE_LOG(LogTemp, Warning, TEXT("Success Rate: %.1f%%"), SuccessRate);
    
    if (SuccessRate >= 90.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ v4.2 DEPLOYMENT VERIFICATION: EXCELLENT"));
    }
    else if (SuccessRate >= 75.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠ v4.2 DEPLOYMENT VERIFICATION: GOOD (Minor issues detected)"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ v4.2 DEPLOYMENT VERIFICATION: NEEDS ATTENTION (Critical issues detected)"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("=== END VERIFICATION REPORT ==="));
}

TArray<FVerificationTestResult> UV42VerificationTest::GetTestResults() const
{
    return TestResults;
}

bool UV42VerificationTest::AreTestsCompleted() const
{
    return bTestsCompleted;
}

float UV42VerificationTest::GetOverallSuccessRate() const
{
    if (TestResults.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 PassedTests = 0;
    for (const FVerificationTestResult& Result : TestResults)
    {
        if (Result.bPassed)
        {
            PassedTests++;
        }
    }
    
    return (float)PassedTests / (float)TestResults.Num() * 100.0f;
}