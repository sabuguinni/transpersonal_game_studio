#include "Eng_CompilationValidator.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"

void UEng_CompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bValidationInProgress = false;
    TotalValidationTime = 0.0f;
    ValidationResults.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Eng_CompilationValidator initialized"));
    
    // Auto-validate on startup
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &UEng_CompilationValidator::ValidateAllClasses,
        2.0f,
        false
    );
}

void UEng_CompilationValidator::ValidateAllClasses()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation already in progress"));
        return;
    }
    
    bValidationInProgress = true;
    ValidationResults.Empty();
    TotalValidationTime = 0.0f;
    
    double StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("Starting compilation validation..."));
    
    ValidateTranspersonalClasses();
    ValidateEngineClasses();
    
    TotalValidationTime = FPlatformTime::Seconds() - StartTime;
    bValidationInProgress = false;
    
    LogValidationSummary();
}

bool UEng_CompilationValidator::ValidateClass(const FString& ClassPath)
{
    FEng_ClassValidationResult Result = TestClassAvailability(ClassPath);
    ValidationResults.Add(Result);
    
    return Result.Status == EEng_CompilationStatus::Success;
}

TArray<FEng_ClassValidationResult> UEng_CompilationValidator::GetValidationResults() const
{
    return ValidationResults;
}

int32 UEng_CompilationValidator::GetFailedClassCount() const
{
    int32 FailCount = 0;
    for (const FEng_ClassValidationResult& Result : ValidationResults)
    {
        if (Result.Status == EEng_CompilationStatus::Failed)
        {
            FailCount++;
        }
    }
    return FailCount;
}

float UEng_CompilationValidator::GetOverallValidationTime() const
{
    return TotalValidationTime;
}

void UEng_CompilationValidator::ValidateTranspersonalClasses()
{
    TArray<FString> TranspersonalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager"),
        TEXT("/Script/TranspersonalGame.Eng_SystemRegistry"),
        TEXT("/Script/TranspersonalGame.Eng_PerformanceMonitor"),
        TEXT("/Script/TranspersonalGame.Eng_ModuleValidator"),
        TEXT("/Script/TranspersonalGame.Eng_WorldSystemManager"),
        TEXT("/Script/TranspersonalGame.Eng_BiomeSystemManager")
    };
    
    for (const FString& ClassPath : TranspersonalClasses)
    {
        ValidateClass(ClassPath);
    }
}

void UEng_CompilationValidator::ValidateEngineClasses()
{
    TArray<FString> EngineClasses = {
        TEXT("/Script/Engine.Actor"),
        TEXT("/Script/Engine.Pawn"),
        TEXT("/Script/Engine.Character"),
        TEXT("/Script/Engine.GameMode"),
        TEXT("/Script/Engine.PlayerController"),
        TEXT("/Script/Engine.GameState"),
        TEXT("/Script/Engine.StaticMeshActor"),
        TEXT("/Script/Engine.DirectionalLight")
    };
    
    for (const FString& ClassPath : EngineClasses)
    {
        ValidateClass(ClassPath);
    }
}

FEng_ClassValidationResult UEng_CompilationValidator::TestClassAvailability(const FString& ClassPath)
{
    FEng_ClassValidationResult Result;
    Result.ClassName = ClassPath;
    
    double StartTime = FPlatformTime::Seconds();
    
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    
    Result.ValidationTime = FPlatformTime::Seconds() - StartTime;
    
    if (LoadedClass)
    {
        Result.Status = EEng_CompilationStatus::Success;
        Result.ErrorMessage = TEXT("Class loaded successfully");
        UE_LOG(LogTemp, Log, TEXT("✓ %s - OK"), *ClassPath);
    }
    else
    {
        Result.Status = EEng_CompilationStatus::Failed;
        Result.ErrorMessage = TEXT("Class not found or failed to load");
        UE_LOG(LogTemp, Error, TEXT("✗ %s - FAILED"), *ClassPath);
    }
    
    return Result;
}

void UEng_CompilationValidator::LogValidationSummary()
{
    int32 SuccessCount = 0;
    int32 FailCount = 0;
    
    for (const FEng_ClassValidationResult& Result : ValidationResults)
    {
        if (Result.Status == EEng_CompilationStatus::Success)
        {
            SuccessCount++;
        }
        else
        {
            FailCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== COMPILATION VALIDATION SUMMARY ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Classes Tested: %d"), ValidationResults.Num());
    UE_LOG(LogTemp, Log, TEXT("Successful: %d"), SuccessCount);
    UE_LOG(LogTemp, Log, TEXT("Failed: %d"), FailCount);
    UE_LOG(LogTemp, Log, TEXT("Validation Time: %.3f seconds"), TotalValidationTime);
    UE_LOG(LogTemp, Log, TEXT("====================================="));
}