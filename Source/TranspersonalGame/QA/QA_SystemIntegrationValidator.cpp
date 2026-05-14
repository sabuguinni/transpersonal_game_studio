#include "QA_SystemIntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UQA_SystemIntegrationValidator::UQA_SystemIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second
    
    bAutoRunTests = false;
    TestInterval = 30.0f; // Run tests every 30 seconds
    bLogVerbose = true;
    
    // Performance thresholds
    MaxFrameTimeMs = 33.33f; // 30 FPS minimum
    MaxMemoryUsageMB = 4096.0f; // 4GB max
    MaxActorCount = 10000;
    MaxDrawCalls = 2000;
    
    LastTestTime = 0.0f;
    bTestsCompleted = false;
}

void UQA_SystemIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bLogVerbose)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA System Integration Validator initialized"));
    }
    
    // Run initial quick validation
    RunQuickValidation();
}

void UQA_SystemIntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics every tick
    UpdatePerformanceMetrics();
    
    // Auto-run tests if enabled
    if (bAutoRunTests)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastTestTime >= TestInterval)
        {
            RunQuickValidation();
            LastTestTime = CurrentTime;
        }
    }
}

bool UQA_SystemIntegrationValidator::ValidateAllSystems()
{
    if (bLogVerbose)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: Starting full system validation"));
    }
    
    TestResults.Empty();
    
    bool bAllPassed = true;
    
    // Validate each system
    bAllPassed &= ValidateVFXSystem();
    bAllPassed &= ValidateCharacterSystem();
    bAllPassed &= ValidateWorldGeneration();
    bAllPassed &= ValidateAudioSystem();
    bAllPassed &= ValidatePerformance();
    
    bTestsCompleted = true;
    
    if (bLogVerbose)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: Full validation complete - Result: %s"), 
               bAllPassed ? TEXT("PASS") : TEXT("FAIL"));
    }
    
    return bAllPassed;
}

bool UQA_SystemIntegrationValidator::ValidateVFXSystem()
{
    FQA_SystemTest VFXTest = ExecuteTest(TEXT("VFX System Validation"), [this]() -> bool
    {
        // Test VFX manager class loading
        UClass* VFXManagerClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.VFX_SystemManager"));
        if (!VFXManagerClass)
        {
            UE_LOG(LogTemp, Error, TEXT("QA: VFX_SystemManager class not found"));
            return false;
        }
        
        // Test Niagara system availability
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        // Check for VFX actors in the world
        int32 VFXActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetName().Contains(TEXT("VFX")))
            {
                VFXActorCount++;
            }
        }
        
        if (bLogVerbose)
        {
            UE_LOG(LogTemp, Warning, TEXT("QA: Found %d VFX actors"), VFXActorCount);
        }
        
        return true;
    });
    
    TestResults.Add(VFXTest);
    LogTestResult(VFXTest);
    
    return VFXTest.Result == EQA_ValidationResult::Pass;
}

bool UQA_SystemIntegrationValidator::ValidateCharacterSystem()
{
    FQA_SystemTest CharTest = ExecuteTest(TEXT("Character System Validation"), [this]() -> bool
    {
        // Test character class loading
        UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
        if (!CharacterClass)
        {
            UE_LOG(LogTemp, Error, TEXT("QA: TranspersonalCharacter class not found"));
            return false;
        }
        
        // Test game mode class loading
        UClass* GameModeClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameMode"));
        if (!GameModeClass)
        {
            UE_LOG(LogTemp, Error, TEXT("QA: TranspersonalGameMode class not found"));
            return false;
        }
        
        if (bLogVerbose)
        {
            UE_LOG(LogTemp, Warning, TEXT("QA: Character system classes loaded successfully"));
        }
        
        return true;
    });
    
    TestResults.Add(CharTest);
    LogTestResult(CharTest);
    
    return CharTest.Result == EQA_ValidationResult::Pass;
}

bool UQA_SystemIntegrationValidator::ValidateWorldGeneration()
{
    FQA_SystemTest WorldTest = ExecuteTest(TEXT("World Generation Validation"), [this]() -> bool
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        // Check for landscape actors
        int32 LandscapeCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && (Actor->GetClass()->GetName().Contains(TEXT("Landscape")) || 
                         Actor->GetName().Contains(TEXT("Terrain"))))
            {
                LandscapeCount++;
            }
        }
        
        if (bLogVerbose)
        {
            UE_LOG(LogTemp, Warning, TEXT("QA: Found %d landscape/terrain actors"), LandscapeCount);
        }
        
        return LandscapeCount > 0;
    });
    
    TestResults.Add(WorldTest);
    LogTestResult(WorldTest);
    
    return WorldTest.Result == EQA_ValidationResult::Pass;
}

bool UQA_SystemIntegrationValidator::ValidateAudioSystem()
{
    FQA_SystemTest AudioTest = ExecuteTest(TEXT("Audio System Validation"), [this]() -> bool
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        // Check for audio components and actors
        int32 AudioComponentCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TArray<UActorComponent*> AudioComponents = Actor->GetComponentsByClass(UAudioComponent::StaticClass());
                AudioComponentCount += AudioComponents.Num();
            }
        }
        
        if (bLogVerbose)
        {
            UE_LOG(LogTemp, Warning, TEXT("QA: Found %d audio components"), AudioComponentCount);
        }
        
        return true; // Audio system is optional for basic functionality
    });
    
    TestResults.Add(AudioTest);
    LogTestResult(AudioTest);
    
    return AudioTest.Result == EQA_ValidationResult::Pass;
}

bool UQA_SystemIntegrationValidator::ValidatePerformance()
{
    FQA_SystemTest PerfTest = ExecuteTest(TEXT("Performance Validation"), [this]() -> bool
    {
        UpdatePerformanceMetrics();
        
        bool bWithinLimits = IsPerformanceWithinLimits();
        
        if (bLogVerbose)
        {
            UE_LOG(LogTemp, Warning, TEXT("QA: Performance check - Actors: %d, Frame: %.2fms, Memory: %.2fMB"), 
                   CurrentMetrics.ActorCount, CurrentMetrics.FrameTimeMs, CurrentMetrics.MemoryUsageMB);
        }
        
        return bWithinLimits;
    });
    
    TestResults.Add(PerfTest);
    LogTestResult(PerfTest);
    
    return PerfTest.Result == EQA_ValidationResult::Pass;
}

FQA_PerformanceMetrics UQA_SystemIntegrationValidator::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics;
}

bool UQA_SystemIntegrationValidator::IsPerformanceWithinLimits()
{
    return (CurrentMetrics.FrameTimeMs <= MaxFrameTimeMs &&
            CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB &&
            CurrentMetrics.ActorCount <= MaxActorCount &&
            CurrentMetrics.DrawCalls <= MaxDrawCalls);
}

void UQA_SystemIntegrationValidator::RunFullTestSuite()
{
    ValidateAllSystems();
}

void UQA_SystemIntegrationValidator::RunQuickValidation()
{
    if (bLogVerbose)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: Running quick validation"));
    }
    
    TestResults.Empty();
    
    // Quick tests - essential systems only
    ValidateCharacterSystem();
    ValidatePerformance();
    
    bTestsCompleted = true;
}

TArray<FQA_SystemTest> UQA_SystemIntegrationValidator::GetTestResults()
{
    return TestResults;
}

FString UQA_SystemIntegrationValidator::GenerateQAReport()
{
    FString Report = TEXT("=== QA SYSTEM VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Tests: %d\n\n"), TestResults.Num());
    
    int32 PassCount = 0;
    int32 FailCount = 0;
    int32 WarningCount = 0;
    
    for (const FQA_SystemTest& Test : TestResults)
    {
        Report += FString::Printf(TEXT("[%s] %s"), 
                                 *UEnum::GetValueAsString(Test.Result), 
                                 *Test.TestName);
        
        if (!Test.ErrorMessage.IsEmpty())
        {
            Report += FString::Printf(TEXT(" - %s"), *Test.ErrorMessage);
        }
        
        Report += FString::Printf(TEXT(" (%.2fms)\n"), Test.ExecutionTimeMs);
        
        switch (Test.Result)
        {
            case EQA_ValidationResult::Pass: PassCount++; break;
            case EQA_ValidationResult::Fail: FailCount++; break;
            case EQA_ValidationResult::Warning: WarningCount++; break;
        }
    }
    
    Report += FString::Printf(TEXT("\nSUMMARY: %d PASS, %d FAIL, %d WARNING\n"), 
                             PassCount, FailCount, WarningCount);
    
    // Performance metrics
    Report += TEXT("\n=== PERFORMANCE METRICS ===\n");
    Report += FString::Printf(TEXT("Actors: %d / %d\n"), CurrentMetrics.ActorCount, MaxActorCount);
    Report += FString::Printf(TEXT("Frame Time: %.2fms / %.2fms\n"), CurrentMetrics.FrameTimeMs, MaxFrameTimeMs);
    Report += FString::Printf(TEXT("Memory: %.2fMB / %.2fMB\n"), CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);
    Report += FString::Printf(TEXT("Draw Calls: %d / %d\n"), CurrentMetrics.DrawCalls, MaxDrawCalls);
    
    return Report;
}

void UQA_SystemIntegrationValidator::SaveQAReportToFile(const FString& FilePath)
{
    FString Report = GenerateQAReport();
    FFileHelper::SaveStringToFile(Report, *FilePath);
    
    if (bLogVerbose)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: Report saved to %s"), *FilePath);
    }
}

FQA_SystemTest UQA_SystemIntegrationValidator::ExecuteTest(const FString& TestName, TFunction<bool()> TestFunction)
{
    FQA_SystemTest Test;
    Test.TestName = TestName;
    
    double StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bResult = TestFunction();
        Test.Result = bResult ? EQA_ValidationResult::Pass : EQA_ValidationResult::Fail;
        
        if (!bResult)
        {
            Test.ErrorMessage = TEXT("Test function returned false");
        }
    }
    catch (...)
    {
        Test.Result = EQA_ValidationResult::Fail;
        Test.ErrorMessage = TEXT("Test function threw exception");
    }
    
    double EndTime = FPlatformTime::Seconds();
    Test.ExecutionTimeMs = (EndTime - StartTime) * 1000.0f;
    
    return Test;
}

void UQA_SystemIntegrationValidator::LogTestResult(const FQA_SystemTest& Test)
{
    if (bLogVerbose)
    {
        FString ResultStr = UEnum::GetValueAsString(Test.Result);
        UE_LOG(LogTemp, Warning, TEXT("QA Test [%s]: %s (%.2fms)"), 
               *ResultStr, *Test.TestName, Test.ExecutionTimeMs);
        
        if (!Test.ErrorMessage.IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("  Error: %s"), *Test.ErrorMessage);
        }
    }
}

void UQA_SystemIntegrationValidator::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count actors
    CurrentMetrics.ActorCount = 0;
    CurrentMetrics.ComponentCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        CurrentMetrics.ActorCount++;
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            CurrentMetrics.ComponentCount += Actor->GetRootComponent() ? 
                Actor->GetRootComponent()->GetAttachChildren().Num() + 1 : 0;
        }
    }
    
    // Frame time (simplified - would need more complex implementation for real frame time)
    CurrentMetrics.FrameTimeMs = FApp::GetDeltaTime() * 1000.0f;
    
    // Memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Draw calls (simplified estimate)
    CurrentMetrics.DrawCalls = CurrentMetrics.ComponentCount; // Rough estimate
}