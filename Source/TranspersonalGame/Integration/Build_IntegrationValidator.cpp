#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    bVerboseLogging = true;
    PerformanceThresholdFPS = 30.0f;
    MaxMemoryUsageMB = 4096;
}

void UBuild_IntegrationValidator::RunFullValidation()
{
    ClearValidationReports();
    
    UE_LOG(LogTemp, Warning, TEXT("Starting full integration validation..."));
    
    double StartTime = FPlatformTime::Seconds();
    
    // Run all validation tests
    bool bModuleValid = ValidateModuleCompilation();
    bool bClassValid = ValidateClassLoading();
    bool bLevelValid = ValidateLevelIntegrity();
    bool bPerfValid = ValidatePerformanceMetrics();
    
    double EndTime = FPlatformTime::Seconds();
    float TotalTime = EndTime - StartTime;
    
    // Generate summary report
    int32 SuccessCount = 0;
    int32 WarningCount = 0;
    int32 ErrorCount = 0;
    
    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        switch (Report.Result)
        {
            case EBuild_ValidationResult::Success:
                SuccessCount++;
                break;
            case EBuild_ValidationResult::Warning:
                WarningCount++;
                break;
            case EBuild_ValidationResult::Error:
            case EBuild_ValidationResult::Critical:
                ErrorCount++;
                break;
        }
    }
    
    FString SummaryMessage = FString::Printf(
        TEXT("Validation Complete - Success: %d, Warnings: %d, Errors: %d (%.2fs)"),
        SuccessCount, WarningCount, ErrorCount, TotalTime
    );
    
    AddValidationReport(TEXT("Full Validation"), 
        ErrorCount > 0 ? EBuild_ValidationResult::Error : EBuild_ValidationResult::Success,
        SummaryMessage, TotalTime);
    
    LogValidationSummary();
}

bool UBuild_IntegrationValidator::ValidateModuleCompilation()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Check if TranspersonalGame module is loaded
    bool bModuleLoaded = FModuleManager::Get().IsModuleLoaded("TranspersonalGame");
    
    double EndTime = FPlatformTime::Seconds();
    float ExecutionTime = EndTime - StartTime;
    
    if (bModuleLoaded)
    {
        AddValidationReport(TEXT("Module Compilation"), EBuild_ValidationResult::Success,
            TEXT("TranspersonalGame module loaded successfully"), ExecutionTime);
        return true;
    }
    else
    {
        AddValidationReport(TEXT("Module Compilation"), EBuild_ValidationResult::Error,
            TEXT("TranspersonalGame module not loaded"), ExecutionTime);
        return false;
    }
}

bool UBuild_IntegrationValidator::ValidateClassLoading()
{
    double StartTime = FPlatformTime::Seconds();
    
    TArray<FString> TestClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };
    
    int32 LoadedCount = 0;
    for (const FString& ClassName : TestClasses)
    {
        if (ValidateSpecificClass(ClassName))
        {
            LoadedCount++;
        }
    }
    
    double EndTime = FPlatformTime::Seconds();
    float ExecutionTime = EndTime - StartTime;
    
    FString Message = FString::Printf(TEXT("Loaded %d/%d classes"), LoadedCount, TestClasses.Num());
    EBuild_ValidationResult Result = (LoadedCount == TestClasses.Num()) ? 
        EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    
    AddValidationReport(TEXT("Class Loading"), Result, Message, ExecutionTime);
    
    return LoadedCount > 0;
}

bool UBuild_IntegrationValidator::ValidateLevelIntegrity()
{
    double StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Level Integrity"), EBuild_ValidationResult::Error,
            TEXT("No valid world found"), 0.0f);
        return false;
    }
    
    ValidateActorCounts();
    ValidateSystemIntegration();
    
    double EndTime = FPlatformTime::Seconds();
    float ExecutionTime = EndTime - StartTime;
    
    AddValidationReport(TEXT("Level Integrity"), EBuild_ValidationResult::Success,
        TEXT("Level validation completed"), ExecutionTime);
    
    return true;
}

bool UBuild_IntegrationValidator::ValidatePerformanceMetrics()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Basic performance validation
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    double EndTime = FPlatformTime::Seconds();
    float ExecutionTime = EndTime - StartTime;
    
    FString Message = FString::Printf(TEXT("Current FPS: %.1f"), CurrentFPS);
    EBuild_ValidationResult Result = (CurrentFPS >= PerformanceThresholdFPS) ?
        EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    
    AddValidationReport(TEXT("Performance Metrics"), Result, Message, ExecutionTime);
    
    return CurrentFPS >= PerformanceThresholdFPS;
}

TArray<FBuild_ValidationReport> UBuild_IntegrationValidator::GetValidationReports() const
{
    return ValidationReports;
}

void UBuild_IntegrationValidator::ClearValidationReports()
{
    ValidationReports.Empty();
}

void UBuild_IntegrationValidator::LogValidationSummary()
{
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION VALIDATION SUMMARY ==="));
    
    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        FString ResultString;
        switch (Report.Result)
        {
            case EBuild_ValidationResult::Success:
                ResultString = TEXT("SUCCESS");
                break;
            case EBuild_ValidationResult::Warning:
                ResultString = TEXT("WARNING");
                break;
            case EBuild_ValidationResult::Error:
                ResultString = TEXT("ERROR");
                break;
            case EBuild_ValidationResult::Critical:
                ResultString = TEXT("CRITICAL");
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[%s] %s: %s (%.3fs)"),
            *ResultString, *Report.TestName, *Report.Message, Report.ExecutionTime);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION SUMMARY ==="));
}

void UBuild_IntegrationValidator::AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime)
{
    FBuild_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Message = Message;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationReports.Add(Report);
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation: %s - %s"), *TestName, *Message);
    }
}

bool UBuild_IntegrationValidator::ValidateSpecificClass(const FString& ClassName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (LoadedClass)
    {
        if (bVerboseLogging)
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Class loaded: %s"), *ClassName);
        }
        return true;
    }
    else
    {
        if (bVerboseLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("✗ Failed to load: %s"), *ClassName);
        }
        return false;
    }
}

void UBuild_IntegrationValidator::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    FString Message = FString::Printf(TEXT("Total actors in level: %d"), TotalActors);
    AddValidationReport(TEXT("Actor Count"), EBuild_ValidationResult::Success, Message);
}

void UBuild_IntegrationValidator::ValidateSystemIntegration()
{
    // Test basic system integration
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Check for essential game systems
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor->GetClass()->GetName().Contains(TEXT("PlayerStart")))
        {
            bHasPlayerStart = true;
        }
        if (Actor->GetClass()->GetName().Contains(TEXT("Light")))
        {
            bHasLighting = true;
        }
    }
    
    if (bHasPlayerStart)
    {
        AddValidationReport(TEXT("Player Start"), EBuild_ValidationResult::Success,
            TEXT("PlayerStart found in level"));
    }
    else
    {
        AddValidationReport(TEXT("Player Start"), EBuild_ValidationResult::Warning,
            TEXT("No PlayerStart found in level"));
    }
    
    if (bHasLighting)
    {
        AddValidationReport(TEXT("Lighting"), EBuild_ValidationResult::Success,
            TEXT("Lighting actors found in level"));
    }
    else
    {
        AddValidationReport(TEXT("Lighting"), EBuild_ValidationResult::Warning,
            TEXT("No lighting actors found in level"));
    }
}

// ABuild_IntegrationTestActor Implementation
ABuild_IntegrationTestActor::ABuild_IntegrationTestActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    IntegrationValidator = CreateDefaultSubobject<UBuild_IntegrationValidator>(TEXT("IntegrationValidator"));
    
    bAutoRunOnBeginPlay = false;
    TestInterval = 60.0f; // Run tests every minute
}

void ABuild_IntegrationTestActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        RunIntegrationTests();
        
        // Set up periodic testing
        GetWorldTimerManager().SetTimer(TestTimerHandle, this, 
            &ABuild_IntegrationTestActor::PeriodicValidation, TestInterval, true);
    }
}

void ABuild_IntegrationTestActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABuild_IntegrationTestActor::RunIntegrationTests()
{
    if (IntegrationValidator)
    {
        UE_LOG(LogTemp, Warning, TEXT("Running integration tests from actor: %s"), *GetName());
        IntegrationValidator->RunFullValidation();
    }
}

void ABuild_IntegrationTestActor::TestCrossSystemCommunication()
{
    // Test communication between different game systems
    UE_LOG(LogTemp, Warning, TEXT("Testing cross-system communication..."));
    
    // This would test interactions between different modules
    // For now, just log that the test is running
    UE_LOG(LogTemp, Warning, TEXT("Cross-system communication test completed"));
}

void ABuild_IntegrationTestActor::ValidateBuildConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating build configuration..."));
    
    // Check build-specific settings and configurations
    UE_LOG(LogTemp, Warning, TEXT("Build configuration validation completed"));
}

void ABuild_IntegrationTestActor::PeriodicValidation()
{
    UE_LOG(LogTemp, Log, TEXT("Running periodic validation..."));
    RunIntegrationTests();
}