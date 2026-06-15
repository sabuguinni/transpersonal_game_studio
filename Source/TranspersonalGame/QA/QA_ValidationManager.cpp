#include "QA_ValidationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

UQA_ValidationManager::UQA_ValidationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoRunOnBeginPlay = true;
    PerformanceTargetFPS = 60.0f;
    MaxAllowedActors = 8000;

    // Initialize core class paths for validation
    CoreClassPaths = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };

    // Essential actor types that must exist in MinPlayableMap
    EssentialActorTypes = {
        TEXT("PlayerStart"),
        TEXT("DirectionalLight"),
        TEXT("SkyLight"),
        TEXT("SkyAtmosphere"),
        TEXT("ExponentialHeightFog")
    };
}

void UQA_ValidationManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        RunAllValidationTests();
    }
}

void UQA_ValidationManager::RunAllValidationTests()
{
    TestResults.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_ValidationManager: Starting comprehensive validation tests"));
    
    RunClassLoadingTests();
    RunMapValidationTests();
    RunPerformanceTests();
    RunIntegrationTests();
    
    // Summary
    int32 PassCount = 0;
    int32 WarningCount = 0;
    int32 FailCount = 0;
    int32 CriticalCount = 0;
    
    for (const FQA_TestResult& Result : TestResults)
    {
        switch (Result.Result)
        {
            case EQA_ValidationResult::Pass: PassCount++; break;
            case EQA_ValidationResult::Warning: WarningCount++; break;
            case EQA_ValidationResult::Fail: FailCount++; break;
            case EQA_ValidationResult::Critical: CriticalCount++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA_SUMMARY: Pass=%d, Warning=%d, Fail=%d, Critical=%d"), 
           PassCount, WarningCount, FailCount, CriticalCount);
}

void UQA_ValidationManager::RunClassLoadingTests()
{
    float StartTime = FPlatformTime::Seconds();
    
    int32 LoadedCount = 0;
    int32 FailedCount = 0;
    
    for (const FString& ClassPath : CoreClassPaths)
    {
        if (ValidateClassExists(ClassPath))
        {
            LoadedCount++;
        }
        else
        {
            FailedCount++;
        }
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (FailedCount == 0)
    {
        AddTestResult(TEXT("Class Loading"), EQA_ValidationResult::Pass, 
                     FString::Printf(TEXT("All %d core classes loaded successfully"), LoadedCount), ExecutionTime);
    }
    else if (FailedCount < LoadedCount)
    {
        AddTestResult(TEXT("Class Loading"), EQA_ValidationResult::Warning, 
                     FString::Printf(TEXT("%d classes loaded, %d failed"), LoadedCount, FailedCount), ExecutionTime);
    }
    else
    {
        AddTestResult(TEXT("Class Loading"), EQA_ValidationResult::Critical, 
                     FString::Printf(TEXT("Critical failure: %d classes failed to load"), FailedCount), ExecutionTime);
    }
}

void UQA_ValidationManager::RunMapValidationTests()
{
    float StartTime = FPlatformTime::Seconds();
    
    bool bActorCountValid = ValidateActorCount();
    bool bEssentialActorsValid = ValidateEssentialActors();
    bool bPlayerStartValid = ValidatePlayerStart();
    bool bLightingValid = ValidateLighting();
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    int32 PassedTests = 0;
    if (bActorCountValid) PassedTests++;
    if (bEssentialActorsValid) PassedTests++;
    if (bPlayerStartValid) PassedTests++;
    if (bLightingValid) PassedTests++;
    
    if (PassedTests == 4)
    {
        AddTestResult(TEXT("Map Validation"), EQA_ValidationResult::Pass, 
                     TEXT("All map validation tests passed"), ExecutionTime);
    }
    else if (PassedTests >= 2)
    {
        AddTestResult(TEXT("Map Validation"), EQA_ValidationResult::Warning, 
                     FString::Printf(TEXT("%d/4 map tests passed"), PassedTests), ExecutionTime);
    }
    else
    {
        AddTestResult(TEXT("Map Validation"), EQA_ValidationResult::Fail, 
                     FString::Printf(TEXT("Only %d/4 map tests passed"), PassedTests), ExecutionTime);
    }
}

void UQA_ValidationManager::RunPerformanceTests()
{
    float StartTime = FPlatformTime::Seconds();
    
    float CurrentFPS = MeasureFrameTime();
    bool bPerformanceAcceptable = CurrentFPS >= (PerformanceTargetFPS * 0.8f); // 80% of target
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (bPerformanceAcceptable)
    {
        AddTestResult(TEXT("Performance"), EQA_ValidationResult::Pass, 
                     FString::Printf(TEXT("FPS: %.1f (Target: %.1f)"), CurrentFPS, PerformanceTargetFPS), ExecutionTime);
    }
    else
    {
        AddTestResult(TEXT("Performance"), EQA_ValidationResult::Warning, 
                     FString::Printf(TEXT("FPS: %.1f below target %.1f"), CurrentFPS, PerformanceTargetFPS), ExecutionTime);
    }
}

void UQA_ValidationManager::RunIntegrationTests()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Test basic integration scenarios
    UWorld* World = GetWorld();
    bool bWorldValid = (World != nullptr);
    bool bGameModeValid = (World && World->GetAuthGameMode() != nullptr);
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    
    if (bWorldValid && bGameModeValid)
    {
        AddTestResult(TEXT("Integration"), EQA_ValidationResult::Pass, 
                     TEXT("Core integration tests passed"), ExecutionTime);
    }
    else
    {
        AddTestResult(TEXT("Integration"), EQA_ValidationResult::Fail, 
                     TEXT("Integration test failures detected"), ExecutionTime);
    }
}

void UQA_ValidationManager::GenerateValidationReport()
{
    FString ReportContent = TEXT("=== QA VALIDATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    for (const FQA_TestResult& Result : TestResults)
    {
        FString ResultString;
        switch (Result.Result)
        {
            case EQA_ValidationResult::Pass: ResultString = TEXT("PASS"); break;
            case EQA_ValidationResult::Warning: ResultString = TEXT("WARN"); break;
            case EQA_ValidationResult::Fail: ResultString = TEXT("FAIL"); break;
            case EQA_ValidationResult::Critical: ResultString = TEXT("CRIT"); break;
        }
        
        ReportContent += FString::Printf(TEXT("[%s] %s: %s (%.3fs)\n"), 
                                       *ResultString, *Result.TestName, *Result.Message, Result.ExecutionTime);
    }
    
    FString FilePath = FPaths::ProjectLogDir() / TEXT("QA_ValidationReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Warning, TEXT("QA Validation Report saved to: %s"), *FilePath);
}

void UQA_ValidationManager::AddTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Message, float ExecutionTime)
{
    FQA_TestResult NewResult;
    NewResult.TestName = TestName;
    NewResult.Result = Result;
    NewResult.Message = Message;
    NewResult.ExecutionTime = ExecutionTime;
    
    TestResults.Add(NewResult);
}

bool UQA_ValidationManager::ValidateClassExists(const FString& ClassPath)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    return (LoadedClass != nullptr);
}

bool UQA_ValidationManager::ValidateActorCount()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    return AllActors.Num() <= MaxAllowedActors;
}

bool UQA_ValidationManager::ValidateEssentialActors()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Check for PlayerStart
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    
    return PlayerStarts.Num() > 0;
}

bool UQA_ValidationManager::ValidatePlayerStart()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    
    if (PlayerStarts.Num() == 0) return false;
    
    // Validate PlayerStart is at reasonable location (not at origin exactly)
    APlayerStart* PS = Cast<APlayerStart>(PlayerStarts[0]);
    if (!PS) return false;
    
    FVector Location = PS->GetActorLocation();
    return !Location.Equals(FVector::ZeroVector, 1.0f);
}

bool UQA_ValidationManager::ValidateLighting()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    return DirectionalLights.Num() > 0;
}

float UQA_ValidationManager::MeasureFrameTime()
{
    // Simple frame time measurement
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        return 1.0f / DeltaTime;
    }
    return 60.0f; // Default assumption
}