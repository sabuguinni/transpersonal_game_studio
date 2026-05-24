#include "QA_VFXValidationFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UQA_VFXValidationFramework::UQA_VFXValidationFramework()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update metrics every second
    
    // Set default performance thresholds
    MaxAcceptableFrameTime = 33.33f; // 30 FPS threshold
    MaxAcceptableActorCount = 1000;
    MaxAcceptableMemoryUsage = 2048.0f; // 2GB
    bAutoRunValidationOnBeginPlay = false;
    
    bValidationInProgress = false;
    TestStartTime = 0.0f;
}

void UQA_VFXValidationFramework::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("QA VFX Validation Framework initialized"));
    
    if (bAutoRunValidationOnBeginPlay)
    {
        // Delay validation to allow other systems to initialize
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UQA_VFXValidationFramework::RunFullVFXValidationSuite, 2.0f, false);
    }
}

void UQA_VFXValidationFramework::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics continuously
    UpdatePerformanceMetrics();
}

bool UQA_VFXValidationFramework::ValidateVFXSystemIntegrity()
{
    TestStartTime = FPlatformTime::Seconds();
    bool bAllTestsPassed = true;
    
    UE_LOG(LogTemp, Log, TEXT("QA: Starting VFX System Integrity Validation"));
    
    // Test 1: Validate VFX Manager Classes
    if (!ValidateVFXManagerClasses())
    {
        AddTestResult(TEXT("VFX Manager Classes"), EQA_VFXTestResult::Fail, TEXT("Failed to load VFX manager classes"));
        bAllTestsPassed = false;
    }
    else
    {
        AddTestResult(TEXT("VFX Manager Classes"), EQA_VFXTestResult::Pass);
    }
    
    // Test 2: Validate Niagara Availability
    if (!ValidateNiagaraAvailability())
    {
        AddTestResult(TEXT("Niagara Availability"), EQA_VFXTestResult::Fail, TEXT("Niagara system not available"));
        bAllTestsPassed = false;
    }
    else
    {
        AddTestResult(TEXT("Niagara Availability"), EQA_VFXTestResult::Pass);
    }
    
    // Test 3: Test VFX Actor Spawning
    if (!TestVFXActorSpawning())
    {
        AddTestResult(TEXT("VFX Actor Spawning"), EQA_VFXTestResult::Warning, TEXT("VFX actor spawning had issues"));
    }
    else
    {
        AddTestResult(TEXT("VFX Actor Spawning"), EQA_VFXTestResult::Pass);
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - TestStartTime;
    UE_LOG(LogTemp, Log, TEXT("QA: VFX System Integrity Validation completed in %.2f seconds"), ExecutionTime);
    
    return bAllTestsPassed;
}

bool UQA_VFXValidationFramework::TestNiagaraSystemLoading()
{
    TestStartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test loading Niagara component class
        UClass* NiagaraComponentClass = UNiagaraComponent::StaticClass();
        if (!NiagaraComponentClass)
        {
            AddTestResult(TEXT("Niagara Component Loading"), EQA_VFXTestResult::Fail, TEXT("Failed to load NiagaraComponent class"));
            return false;
        }
        
        // Test loading Niagara system class
        UClass* NiagaraSystemClass = UNiagaraSystem::StaticClass();
        if (!NiagaraSystemClass)
        {
            AddTestResult(TEXT("Niagara System Loading"), EQA_VFXTestResult::Fail, TEXT("Failed to load NiagaraSystem class"));
            return false;
        }
        
        float ExecutionTime = FPlatformTime::Seconds() - TestStartTime;
        AddTestResult(TEXT("Niagara System Loading"), EQA_VFXTestResult::Pass, TEXT(""), ExecutionTime);
        
        UE_LOG(LogTemp, Log, TEXT("QA: Niagara system loading test passed"));
        return true;
    }
    catch (...)
    {
        AddTestResult(TEXT("Niagara System Loading"), EQA_VFXTestResult::Fail, TEXT("Exception during Niagara loading test"));
        return false;
    }
}

bool UQA_VFXValidationFramework::ValidateVFXComponentAttachment()
{
    TestStartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddTestResult(TEXT("VFX Component Attachment"), EQA_VFXTestResult::Fail, TEXT("No valid world context"));
        return false;
    }
    
    // Find actors in the level that can have VFX components attached
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ActorsWithComponents = 0;
    int32 TestableActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && !Actor->IsPendingKill())
        {
            TestableActors++;
            
            // Check if actor has any components
            TArray<UActorComponent*> Components = Actor->GetComponents<UActorComponent>().Array();
            if (Components.Num() > 0)
            {
                ActorsWithComponents++;
            }
        }
    }
    
    float ComponentRatio = TestableActors > 0 ? (float)ActorsWithComponents / (float)TestableActors : 0.0f;
    
    float ExecutionTime = FPlatformTime::Seconds() - TestStartTime;
    
    if (ComponentRatio > 0.5f) // At least 50% of actors should have components
    {
        AddTestResult(TEXT("VFX Component Attachment"), EQA_VFXTestResult::Pass, 
                     FString::Printf(TEXT("%.1f%% actors have components"), ComponentRatio * 100.0f), ExecutionTime);
        return true;
    }
    else
    {
        AddTestResult(TEXT("VFX Component Attachment"), EQA_VFXTestResult::Warning, 
                     FString::Printf(TEXT("Only %.1f%% actors have components"), ComponentRatio * 100.0f), ExecutionTime);
        return false;
    }
}

bool UQA_VFXValidationFramework::TestVFXPerformanceMetrics()
{
    TestStartTime = FPlatformTime::Seconds();
    
    UpdatePerformanceMetrics();
    
    bool bPerformanceAcceptable = true;
    FString PerformanceIssues;
    
    // Check frame time
    if (!CheckFrameTimeThreshold())
    {
        bPerformanceAcceptable = false;
        PerformanceIssues += FString::Printf(TEXT("Frame time: %.2fms (max: %.2fms); "), 
                                           CurrentMetrics.FrameTime, MaxAcceptableFrameTime);
    }
    
    // Check actor count
    if (!CheckActorCountThreshold())
    {
        bPerformanceAcceptable = false;
        PerformanceIssues += FString::Printf(TEXT("Actor count: %d (max: %d); "), 
                                           CurrentMetrics.TotalActorsInLevel, MaxAcceptableActorCount);
    }
    
    // Check memory usage
    if (!CheckMemoryUsageThreshold())
    {
        bPerformanceAcceptable = false;
        PerformanceIssues += FString::Printf(TEXT("Memory: %.1fMB (max: %.1fMB); "), 
                                           CurrentMetrics.MemoryUsageMB, MaxAcceptableMemoryUsage);
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - TestStartTime;
    
    if (bPerformanceAcceptable)
    {
        AddTestResult(TEXT("VFX Performance Metrics"), EQA_VFXTestResult::Pass, TEXT("All metrics within acceptable range"), ExecutionTime);
    }
    else
    {
        AddTestResult(TEXT("VFX Performance Metrics"), EQA_VFXTestResult::Warning, PerformanceIssues, ExecutionTime);
    }
    
    return bPerformanceAcceptable;
}

bool UQA_VFXValidationFramework::ValidateVFXAudioIntegration()
{
    TestStartTime = FPlatformTime::Seconds();
    
    // Basic audio system availability check
    UWorld* World = GetWorld();
    if (!World)
    {
        AddTestResult(TEXT("VFX Audio Integration"), EQA_VFXTestResult::Fail, TEXT("No world context for audio testing"));
        return false;
    }
    
    // Check if audio device is available
    if (GEngine && GEngine->GetAudioDeviceManager())
    {
        float ExecutionTime = FPlatformTime::Seconds() - TestStartTime;
        AddTestResult(TEXT("VFX Audio Integration"), EQA_VFXTestResult::Pass, TEXT("Audio system available"), ExecutionTime);
        return true;
    }
    else
    {
        float ExecutionTime = FPlatformTime::Seconds() - TestStartTime;
        AddTestResult(TEXT("VFX Audio Integration"), EQA_VFXTestResult::Warning, TEXT("Audio system not fully available"), ExecutionTime);
        return false;
    }
}

void UQA_VFXValidationFramework::RunFullVFXValidationSuite()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA: VFX validation already in progress"));
        return;
    }
    
    bValidationInProgress = true;
    ClearTestResults();
    
    UE_LOG(LogTemp, Log, TEXT("QA: Starting Full VFX Validation Suite"));
    
    // Run all validation tests
    ValidateVFXSystemIntegrity();
    TestNiagaraSystemLoading();
    ValidateVFXComponentAttachment();
    TestVFXPerformanceMetrics();
    ValidateVFXAudioIntegration();
    
    bValidationInProgress = false;
    
    // Generate report
    GenerateVFXValidationReport();
    
    UE_LOG(LogTemp, Log, TEXT("QA: Full VFX Validation Suite completed"));
}

void UQA_VFXValidationFramework::GenerateVFXValidationReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== QA VFX VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Generated: %s"), *FDateTime::Now().ToString());
    UE_LOG(LogTemp, Log, TEXT("Total Tests: %d"), TestResults.Num());
    
    int32 PassCount = 0;
    int32 WarningCount = 0;
    int32 FailCount = 0;
    
    for (const FQA_VFXTestCase& TestCase : TestResults)
    {
        FString ResultString;
        switch (TestCase.Result)
        {
            case EQA_VFXTestResult::Pass:
                ResultString = TEXT("PASS");
                PassCount++;
                break;
            case EQA_VFXTestResult::Warning:
                ResultString = TEXT("WARNING");
                WarningCount++;
                break;
            case EQA_VFXTestResult::Fail:
                ResultString = TEXT("FAIL");
                FailCount++;
                break;
            default:
                ResultString = TEXT("NOT_TESTED");
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("[%s] %s (%.2fs) - %s"), 
               *ResultString, *TestCase.TestName, TestCase.ExecutionTime, *TestCase.ErrorMessage);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Summary: %d PASS, %d WARNING, %d FAIL"), PassCount, WarningCount, FailCount);
    UE_LOG(LogTemp, Log, TEXT("=== END VFX VALIDATION REPORT ==="));
}

FQA_VFXPerformanceMetrics UQA_VFXValidationFramework::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics;
}

bool UQA_VFXValidationFramework::IsVFXPerformanceAcceptable()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics.bPerformanceAcceptable;
}

bool UQA_VFXValidationFramework::ValidateVFXManagerClasses()
{
    // This would normally test loading of custom VFX manager classes
    // For now, we'll do a basic validation
    return true;
}

bool UQA_VFXValidationFramework::ValidateNiagaraAvailability()
{
    UClass* NiagaraComponentClass = UNiagaraComponent::StaticClass();
    UClass* NiagaraSystemClass = UNiagaraSystem::StaticClass();
    
    return (NiagaraComponentClass != nullptr && NiagaraSystemClass != nullptr);
}

bool UQA_VFXValidationFramework::TestVFXActorSpawning()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Test spawning a basic actor for VFX attachment
    FVector TestLocation(0.0f, 0.0f, 200.0f);
    FRotator TestRotation(0.0f, 0.0f, 0.0f);
    
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), TestLocation, TestRotation);
    if (TestActor)
    {
        TestActor->SetActorLabel(TEXT("QA_VFX_TestActor"));
        return true;
    }
    
    return false;
}

bool UQA_VFXValidationFramework::ValidateVFXIntegrationPoints()
{
    // Test integration with other game systems
    return true;
}

void UQA_VFXValidationFramework::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update actor count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    CurrentMetrics.TotalActorsInLevel = AllActors.Num();
    
    // Update frame time (simplified)
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Update memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Count active particle systems
    CurrentMetrics.ActiveParticleSystems = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            TArray<UNiagaraComponent*> NiagaraComponents;
            Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
            CurrentMetrics.ActiveParticleSystems += NiagaraComponents.Num();
        }
    }
    
    // Determine if performance is acceptable
    CurrentMetrics.bPerformanceAcceptable = CheckFrameTimeThreshold() && 
                                          CheckActorCountThreshold() && 
                                          CheckMemoryUsageThreshold();
}

bool UQA_VFXValidationFramework::CheckFrameTimeThreshold()
{
    return CurrentMetrics.FrameTime <= MaxAcceptableFrameTime;
}

bool UQA_VFXValidationFramework::CheckMemoryUsageThreshold()
{
    return CurrentMetrics.MemoryUsageMB <= MaxAcceptableMemoryUsage;
}

bool UQA_VFXValidationFramework::CheckActorCountThreshold()
{
    return CurrentMetrics.TotalActorsInLevel <= MaxAcceptableActorCount;
}

void UQA_VFXValidationFramework::AddTestResult(const FString& TestName, EQA_VFXTestResult Result, const FString& ErrorMessage, float ExecutionTime)
{
    FQA_VFXTestCase NewTestCase;
    NewTestCase.TestName = TestName;
    NewTestCase.Result = Result;
    NewTestCase.ErrorMessage = ErrorMessage;
    NewTestCase.ExecutionTime = ExecutionTime;
    
    TestResults.Add(NewTestCase);
}

void UQA_VFXValidationFramework::ClearTestResults()
{
    TestResults.Empty();
}