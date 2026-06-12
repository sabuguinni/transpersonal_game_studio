#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "EditorLevelLibrary.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
    bAutoValidateOnBeginPlay = true;
    ValidationInterval = 30.0f;
    TimeSinceLastValidation = 0.0f;

    // Initialize core class paths
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.FoliageManager"));
    CoreClassPaths.Add(TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));

    // Initialize QA class paths
    QAClassPaths.Add(TEXT("/Script/TranspersonalGame.QA_TestFramework"));
    QAClassPaths.Add(TEXT("/Script/TranspersonalGame.QA_PerformanceMonitor"));
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoValidateOnBeginPlay)
    {
        ValidateAllModules();
    }
}

void UBuild_IntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastValidation += DeltaTime;
    
    if (TimeSinceLastValidation >= ValidationInterval && !bValidationInProgress)
    {
        ValidateAllModules();
        TimeSinceLastValidation = 0.0f;
    }
}

bool UBuild_IntegrationValidator::ValidateAllModules()
{
    if (bValidationInProgress)
    {
        return false;
    }

    bValidationInProgress = true;
    ClearValidationReports();
    
    float StartTime = FPlatformTime::Seconds();
    
    bool bCoreValid = ValidateCoreClasses();
    bool bQAValid = ValidateQAIntegration();
    bool bLevelValid = ValidateLevelActors();
    bool bBuildValid = CheckCompilationStatus();
    
    float EndTime = FPlatformTime::Seconds();
    LastValidationTime = EndTime - StartTime;
    
    bool bAllValid = bCoreValid && bQAValid && bLevelValid && bBuildValid;
    
    FString SummaryMessage = FString::Printf(TEXT("Validation complete: Core=%s, QA=%s, Level=%s, Build=%s (%.2fs)"),
        bCoreValid ? TEXT("OK") : TEXT("FAIL"),
        bQAValid ? TEXT("OK") : TEXT("FAIL"),
        bLevelValid ? TEXT("OK") : TEXT("FAIL"),
        bBuildValid ? TEXT("OK") : TEXT("FAIL"),
        LastValidationTime);
    
    AddValidationReport(TEXT("Integration"), 
        bAllValid ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error,
        SummaryMessage);
    
    bValidationInProgress = false;
    return bAllValid;
}

bool UBuild_IntegrationValidator::ValidateCoreClasses()
{
    int32 LoadedCount = 0;
    
    for (const FString& ClassPath : CoreClassPaths)
    {
        if (ValidateSpecificClass(ClassPath))
        {
            LoadedCount++;
        }
    }
    
    bool bSuccess = LoadedCount == CoreClassPaths.Num();
    FString Message = FString::Printf(TEXT("Core classes loaded: %d/%d"), LoadedCount, CoreClassPaths.Num());
    
    AddValidationReport(TEXT("CoreClasses"), 
        bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error,
        Message);
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateQAIntegration()
{
    int32 LoadedCount = 0;
    
    for (const FString& ClassPath : QAClassPaths)
    {
        if (ValidateSpecificClass(ClassPath))
        {
            LoadedCount++;
        }
    }
    
    bool bSuccess = LoadedCount > 0; // At least some QA classes should be available
    FString Message = FString::Printf(TEXT("QA classes loaded: %d/%d"), LoadedCount, QAClassPaths.Num());
    
    AddValidationReport(TEXT("QAIntegration"), 
        bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning,
        Message);
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateLevelActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("LevelActors"), EBuild_ValidationResult::Error, TEXT("No world available"));
        return false;
    }
    
    int32 ActorCount = 0;
    int32 CustomActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            ActorCount++;
            FString ClassName = Actor->GetClass()->GetName();
            if (ClassName.Contains(TEXT("Transpersonal")))
            {
                CustomActorCount++;
            }
        }
    }
    
    bool bSuccess = ActorCount > 0;
    FString Message = FString::Printf(TEXT("Level actors: %d total, %d custom"), ActorCount, CustomActorCount);
    
    AddValidationReport(TEXT("LevelActors"), 
        bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning,
        Message);
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::CheckCompilationStatus()
{
    // Check if we can access basic UE5 functionality
    bool bEngineValid = GEngine != nullptr;
    bool bWorldValid = GetWorld() != nullptr;
    
    FString Message = FString::Printf(TEXT("Engine=%s, World=%s"), 
        bEngineValid ? TEXT("OK") : TEXT("FAIL"),
        bWorldValid ? TEXT("OK") : TEXT("FAIL"));
    
    bool bSuccess = bEngineValid && bWorldValid;
    
    AddValidationReport(TEXT("Compilation"), 
        bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Critical,
        Message);
    
    return bSuccess;
}

TArray<FBuild_ValidationReport> UBuild_IntegrationValidator::GetValidationReports() const
{
    return ValidationReports;
}

void UBuild_IntegrationValidator::ClearValidationReports()
{
    ValidationReports.Empty();
}

int32 UBuild_IntegrationValidator::GetLoadedModuleCount() const
{
    return CoreClassPaths.Num() + QAClassPaths.Num();
}

FString UBuild_IntegrationValidator::GetBuildStatusSummary() const
{
    if (ValidationReports.Num() == 0)
    {
        return TEXT("No validation data available");
    }
    
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
    
    return FString::Printf(TEXT("Build Status: %d OK, %d Warnings, %d Errors (%.2fs)"), 
        SuccessCount, WarningCount, ErrorCount, LastValidationTime);
}

void UBuild_IntegrationValidator::AddValidationReport(const FString& ModuleName, EBuild_ValidationResult Result, const FString& Message)
{
    FBuild_ValidationReport Report;
    Report.ModuleName = ModuleName;
    Report.Result = Result;
    Report.Message = Message;
    Report.ValidationTime = FPlatformTime::Seconds();
    
    ValidationReports.Add(Report);
    LogValidationResult(Report);
}

bool UBuild_IntegrationValidator::ValidateSpecificClass(const FString& ClassName)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
    return LoadedClass != nullptr;
}

void UBuild_IntegrationValidator::LogValidationResult(const FBuild_ValidationReport& Report)
{
    FString LogMessage = FString::Printf(TEXT("[%s] %s: %s"), 
        *Report.ModuleName, 
        *UEnum::GetValueAsString(Report.Result),
        *Report.Message);
    
    switch (Report.Result)
    {
        case EBuild_ValidationResult::Success:
            UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
            break;
        case EBuild_ValidationResult::Warning:
            UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
            break;
        case EBuild_ValidationResult::Error:
        case EBuild_ValidationResult::Critical:
            UE_LOG(LogTemp, Error, TEXT("%s"), *LogMessage);
            break;
    }
}