#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoRunOnBeginPlay = false;
    bLogDetailedResults = true;
    PerformanceThresholdMS = 16.67f; // 60 FPS target
    TotalActorCount = 0;
    DinosaurActorCount = 0;
    LastValidationTime = 0.0f;

    // Initialize core class paths
    CoreClassPaths = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoRunOnBeginPlay)
    {
        RunFullValidationSuite();
    }
}

bool UBuild_IntegrationValidator::ValidateModuleCompilation()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;

    UE_LOG(LogTemp, Warning, TEXT("=== MODULE COMPILATION VALIDATION ==="));

    // Test core class loading
    int32 LoadedClasses = 0;
    int32 FailedClasses = 0;

    for (const FString& ClassPath : CoreClassPaths)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            LoadedClasses++;
            UE_LOG(LogTemp, Log, TEXT("✓ %s - LOADED"), *ClassPath);
        }
        else
        {
            FailedClasses++;
            bSuccess = false;
            UE_LOG(LogTemp, Error, TEXT("✗ %s - FAILED"), *ClassPath);
        }
    }

    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    FString ResultMessage = FString::Printf(TEXT("Loaded %d/%d core classes"), LoadedClasses, CoreClassPaths.Num());
    
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    AddValidationReport(TEXT("Module Compilation"), Result, ResultMessage, ExecutionTime);

    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateCrossSystemIntegration()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;

    UE_LOG(LogTemp, Warning, TEXT("=== CROSS-SYSTEM INTEGRATION VALIDATION ==="));

    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Cross-System Integration"), EBuild_ValidationResult::Critical, TEXT("No world context available"), 0.0f);
        return false;
    }

    // Test character-world integration
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    
    if (PlayerStarts.Num() == 0)
    {
        bSuccess = false;
        UE_LOG(LogTemp, Error, TEXT("✗ No PlayerStart actors found"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("✓ PlayerStart actors: %d"), PlayerStarts.Num());
    }

    // Test character class availability
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (CharacterClass)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ TranspersonalCharacter class available"));
    }
    else
    {
        bSuccess = false;
        UE_LOG(LogTemp, Error, TEXT("✗ TranspersonalCharacter class missing"));
    }

    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    FString ResultMessage = FString::Printf(TEXT("PlayerStarts: %d, Character class: %s"), 
        PlayerStarts.Num(), CharacterClass ? TEXT("Available") : TEXT("Missing"));
    
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    AddValidationReport(TEXT("Cross-System Integration"), Result, ResultMessage, ExecutionTime);

    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidatePerformanceMetrics()
{
    float StartTime = FPlatformTime::Seconds();

    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE METRICS VALIDATION ==="));

    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("Performance Metrics"), EBuild_ValidationResult::Critical, TEXT("No world context"), 0.0f);
        return false;
    }

    // Count all actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorCount = AllActors.Num();

    // Count static mesh actors
    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);

    // Count dinosaur actors (by label pattern)
    DinosaurActorCount = 0;
    TArray<FString> DinoLabels = {TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), TEXT("ankylo")};
    
    for (AActor* Actor : AllActors)
    {
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        for (const FString& DinoLabel : DinoLabels)
        {
            if (ActorLabel.Contains(DinoLabel))
            {
                DinosaurActorCount++;
                break;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Total actors: %d"), TotalActorCount);
    UE_LOG(LogTemp, Log, TEXT("StaticMeshActors: %d"), StaticMeshActors.Num());
    UE_LOG(LogTemp, Log, TEXT("Dinosaur actors: %d"), DinosaurActorCount);

    // Performance thresholds
    bool bWithinLimits = TotalActorCount <= 8000 && DinosaurActorCount <= 150;
    
    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    FString ResultMessage = FString::Printf(TEXT("Actors: %d/8000, Dinosaurs: %d/150"), TotalActorCount, DinosaurActorCount);
    
    EBuild_ValidationResult Result = bWithinLimits ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    AddValidationReport(TEXT("Performance Metrics"), Result, ResultMessage, ExecutionTime);

    return bWithinLimits;
}

bool UBuild_IntegrationValidator::ValidateVFXSystems()
{
    float StartTime = FPlatformTime::Seconds();

    UE_LOG(LogTemp, Warning, TEXT("=== VFX SYSTEMS VALIDATION ==="));

    // Test VFX framework integration
    UClass* VFXTestClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.QA_VFXTestFramework"));
    bool bVFXIntegrated = (VFXTestClass != nullptr);

    if (bVFXIntegrated)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ QA_VFXTestFramework integrated"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ QA_VFXTestFramework not found"));
    }

    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    FString ResultMessage = FString::Printf(TEXT("VFX Framework: %s"), bVFXIntegrated ? TEXT("Integrated") : TEXT("Missing"));
    
    EBuild_ValidationResult Result = bVFXIntegrated ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    AddValidationReport(TEXT("VFX Systems"), Result, ResultMessage, ExecutionTime);

    return bVFXIntegrated;
}

bool UBuild_IntegrationValidator::RunFullValidationSuite()
{
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("=== FULL VALIDATION SUITE STARTING ==="));
    
    ClearValidationReports();

    bool bModuleValid = ValidateModuleCompilation();
    bool bIntegrationValid = ValidateCrossSystemIntegration();
    bool bPerformanceValid = ValidatePerformanceMetrics();
    bool bVFXValid = ValidateVFXSystems();

    bool bOverallSuccess = bModuleValid && bIntegrationValid && bPerformanceValid && bVFXValid;

    LastValidationTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;

    FString OverallMessage = FString::Printf(TEXT("Module: %s, Integration: %s, Performance: %s, VFX: %s"),
        bModuleValid ? TEXT("PASS") : TEXT("FAIL"),
        bIntegrationValid ? TEXT("PASS") : TEXT("FAIL"),
        bPerformanceValid ? TEXT("PASS") : TEXT("FAIL"),
        bVFXValid ? TEXT("PASS") : TEXT("FAIL"));

    EBuild_ValidationResult OverallResult = bOverallSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    AddValidationReport(TEXT("Full Validation Suite"), OverallResult, OverallMessage, LastValidationTime);

    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION SUITE COMPLETE: %s ==="), bOverallSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));

    return bOverallSuccess;
}

TArray<FBuild_ValidationReport> UBuild_IntegrationValidator::GetValidationReports() const
{
    return ValidationReports;
}

void UBuild_IntegrationValidator::ClearValidationReports()
{
    ValidationReports.Empty();
}

void UBuild_IntegrationValidator::ExportValidationReport(const FString& FilePath)
{
    FString ReportContent = TEXT("=== INTEGRATION VALIDATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());

    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        FString ResultText;
        switch (Report.Result)
        {
            case EBuild_ValidationResult::Success: ResultText = TEXT("SUCCESS"); break;
            case EBuild_ValidationResult::Warning: ResultText = TEXT("WARNING"); break;
            case EBuild_ValidationResult::Error: ResultText = TEXT("ERROR"); break;
            case EBuild_ValidationResult::Critical: ResultText = TEXT("CRITICAL"); break;
        }

        ReportContent += FString::Printf(TEXT("[%s] %s\n"), *ResultText, *Report.TestName);
        ReportContent += FString::Printf(TEXT("  Message: %s\n"), *Report.Message);
        ReportContent += FString::Printf(TEXT("  Execution Time: %.2fms\n\n"), Report.ExecutionTime);
    }

    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
}

void UBuild_IntegrationValidator::AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime)
{
    FBuild_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Message = Message;
    Report.ExecutionTime = ExecutionTime;

    ValidationReports.Add(Report);

    if (bLogDetailedResults)
    {
        FString ResultText;
        switch (Result)
        {
            case EBuild_ValidationResult::Success: ResultText = TEXT("SUCCESS"); break;
            case EBuild_ValidationResult::Warning: ResultText = TEXT("WARNING"); break;
            case EBuild_ValidationResult::Error: ResultText = TEXT("ERROR"); break;
            case EBuild_ValidationResult::Critical: ResultText = TEXT("CRITICAL"); break;
        }

        UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s (%.2fms)"), *ResultText, *TestName, *Message, ExecutionTime);
    }
}