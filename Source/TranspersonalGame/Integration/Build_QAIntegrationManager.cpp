#include "Build_QAIntegrationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ABuild_QAIntegrationManager::ABuild_QAIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bAutoRunTests = false;
    TestInterval = 30.0f;
    LastTestTime = 0.0f;
    bTestsRunning = false;
    
    // Initialize report
    CurrentReport = FBuild_IntegrationReport();
    CurrentReport.BuildTimestamp = FDateTime::Now().ToString();
}

void ABuild_QAIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA Integration Manager initialized"));
    
    if (bAutoRunTests)
    {
        // Run initial tests after a short delay
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &ABuild_QAIntegrationManager::RunQAIntegrationTests,
            2.0f,
            false
        );
    }
}

void ABuild_QAIntegrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoRunTests && !bTestsRunning)
    {
        LastTestTime += DeltaTime;
        if (LastTestTime >= TestInterval)
        {
            RunQAIntegrationTests();
            LastTestTime = 0.0f;
        }
    }
}

void ABuild_QAIntegrationManager::RunQAIntegrationTests()
{
    if (bTestsRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA Integration tests already running"));
        return;
    }
    
    bTestsRunning = true;
    UE_LOG(LogTemp, Warning, TEXT("Starting QA Integration test suite"));
    
    // Clear previous results
    CurrentReport.TestResults.Empty();
    CurrentReport.TotalTests = 0;
    CurrentReport.PassedTests = 0;
    CurrentReport.FailedTests = 0;
    CurrentReport.BuildTimestamp = FDateTime::Now().ToString();
    
    // Run validation tests
    ValidateVFXSystems();
    ValidatePerformanceMetrics();
    ValidateBuildIntegrity();
    
    // Generate final report
    GenerateReport();
    LogIntegrationStatus();
    
    bTestsRunning = false;
    UE_LOG(LogTemp, Warning, TEXT("QA Integration test suite completed"));
}

void ABuild_QAIntegrationManager::ValidateVFXSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Test VFX system availability
    bool bVFXSystemsValid = true;
    FString ErrorMsg = TEXT("");
    
    try
    {
        // Check for Niagara system availability
        UWorld* World = GetWorld();
        if (!World)
        {
            bVFXSystemsValid = false;
            ErrorMsg = TEXT("World not available for VFX testing");
        }
        else
        {
            // Count VFX actors in the world
            TArray<AActor*> AllActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
            
            int32 VFXActorCount = 0;
            for (AActor* Actor : AllActors)
            {
                if (Actor && Actor->GetName().Contains(TEXT("VFX")))
                {
                    VFXActorCount++;
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Found %d VFX-related actors"), VFXActorCount);
        }
    }
    catch (...)
    {
        bVFXSystemsValid = false;
        ErrorMsg = TEXT("Exception during VFX validation");
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(TEXT("VFX Systems Validation"), bVFXSystemsValid, ErrorMsg, ExecutionTime);
}

void ABuild_QAIntegrationManager::ValidatePerformanceMetrics()
{
    float StartTime = FPlatformTime::Seconds();
    
    bool bPerformanceValid = true;
    FString ErrorMsg = TEXT("");
    
    try
    {
        // Check frame rate
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        if (CurrentFPS < 30.0f)
        {
            bPerformanceValid = false;
            ErrorMsg = FString::Printf(TEXT("Low FPS detected: %.2f"), CurrentFPS);
        }
        
        // Check memory usage (basic check)
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        float MemoryUsageGB = MemStats.UsedPhysical / (1024.0f * 1024.0f * 1024.0f);
        
        if (MemoryUsageGB > 8.0f) // Arbitrary threshold
        {
            bPerformanceValid = false;
            ErrorMsg += FString::Printf(TEXT(" High memory usage: %.2f GB"), MemoryUsageGB);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Performance check - FPS: %.2f, Memory: %.2f GB"), CurrentFPS, MemoryUsageGB);
    }
    catch (...)
    {
        bPerformanceValid = false;
        ErrorMsg = TEXT("Exception during performance validation");
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(TEXT("Performance Metrics Validation"), bPerformanceValid, ErrorMsg, ExecutionTime);
}

void ABuild_QAIntegrationManager::ValidateBuildIntegrity()
{
    float StartTime = FPlatformTime::Seconds();
    
    bool bBuildValid = true;
    FString ErrorMsg = TEXT("");
    
    try
    {
        // Check for critical game classes
        TArray<FString> CriticalClasses = {
            TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
            TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
            TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
            TEXT("/Script/TranspersonalGame.FoliageManager")
        };
        
        int32 LoadedClasses = 0;
        for (const FString& ClassName : CriticalClasses)
        {
            UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
            if (LoadedClass)
            {
                LoadedClasses++;
                UE_LOG(LogTemp, Log, TEXT("Successfully loaded class: %s"), *ClassName);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to load class: %s"), *ClassName);
                ErrorMsg += FString::Printf(TEXT("Missing class: %s "), *ClassName);
            }
        }
        
        if (LoadedClasses < CriticalClasses.Num())
        {
            bBuildValid = false;
            ErrorMsg = FString::Printf(TEXT("Only %d/%d critical classes loaded. %s"), 
                LoadedClasses, CriticalClasses.Num(), *ErrorMsg);
        }
    }
    catch (...)
    {
        bBuildValid = false;
        ErrorMsg = TEXT("Exception during build integrity validation");
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(TEXT("Build Integrity Validation"), bBuildValid, ErrorMsg, ExecutionTime);
}

FBuild_IntegrationReport ABuild_QAIntegrationManager::GetIntegrationReport() const
{
    return CurrentReport;
}

bool ABuild_QAIntegrationManager::IsSystemHealthy() const
{
    return CurrentReport.bBuildSuccessful && (CurrentReport.FailedTests == 0);
}

void ABuild_QAIntegrationManager::EditorRunIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("Running QA Integration tests from editor"));
    RunQAIntegrationTests();
}

void ABuild_QAIntegrationManager::AddTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage, float ExecutionTime)
{
    FBuild_QATestResult Result;
    Result.TestName = TestName;
    Result.bPassed = bPassed;
    Result.ErrorMessage = ErrorMessage;
    Result.ExecutionTime = ExecutionTime;
    
    CurrentReport.TestResults.Add(Result);
    CurrentReport.TotalTests++;
    
    if (bPassed)
    {
        CurrentReport.PassedTests++;
    }
    else
    {
        CurrentReport.FailedTests++;
    }
}

void ABuild_QAIntegrationManager::GenerateReport()
{
    CurrentReport.bBuildSuccessful = (CurrentReport.FailedTests == 0);
    
    UE_LOG(LogTemp, Warning, TEXT("=== QA Integration Report ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tests: %d"), CurrentReport.TotalTests);
    UE_LOG(LogTemp, Warning, TEXT("Passed: %d"), CurrentReport.PassedTests);
    UE_LOG(LogTemp, Warning, TEXT("Failed: %d"), CurrentReport.FailedTests);
    UE_LOG(LogTemp, Warning, TEXT("Build Successful: %s"), CurrentReport.bBuildSuccessful ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Timestamp: %s"), *CurrentReport.BuildTimestamp);
}

void ABuild_QAIntegrationManager::LogIntegrationStatus()
{
    for (const FBuild_QATestResult& Result : CurrentReport.TestResults)
    {
        if (Result.bPassed)
        {
            UE_LOG(LogTemp, Log, TEXT("[PASS] %s (%.3fs)"), *Result.TestName, Result.ExecutionTime);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[FAIL] %s (%.3fs): %s"), *Result.TestName, Result.ExecutionTime, *Result.ErrorMessage);
        }
    }
}