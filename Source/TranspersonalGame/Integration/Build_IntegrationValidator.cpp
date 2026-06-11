#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    InitializeCriticalClasses();
    InitializeQAClasses();
}

void UBuild_IntegrationValidator::InitializeCriticalClasses()
{
    CriticalClassNames.Empty();
    CriticalClassNames.Add(TEXT("TranspersonalCharacter"));
    CriticalClassNames.Add(TEXT("TranspersonalGameState"));
    CriticalClassNames.Add(TEXT("PCGWorldGenerator"));
    CriticalClassNames.Add(TEXT("FoliageManager"));
    CriticalClassNames.Add(TEXT("CrowdSimulationManager"));
    CriticalClassNames.Add(TEXT("ProceduralWorldManager"));
    CriticalClassNames.Add(TEXT("BuildIntegrationManager"));
}

void UBuild_IntegrationValidator::InitializeQAClasses()
{
    QAClassNames.Empty();
    QAClassNames.Add(TEXT("QA_VFXIntegrationTest"));
    QAClassNames.Add(TEXT("QA_SystemValidator"));
}

FBuild_ValidationResult UBuild_IntegrationValidator::ValidateIntegration()
{
    FBuild_ValidationResult Result;
    Result.PassedChecks = 0;
    Result.TotalChecks = 4;

    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION VALIDATION STARTED ==="));

    // Check 1: Critical Classes
    if (ValidateCriticalClasses())
    {
        Result.PassedChecks++;
        UE_LOG(LogTemp, Warning, TEXT("✓ Critical classes validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Critical classes validation FAILED"));
    }

    // Check 2: Binary Compilation
    if (ValidateBinaryCompilation())
    {
        Result.PassedChecks++;
        UE_LOG(LogTemp, Warning, TEXT("✓ Binary compilation validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Binary compilation validation FAILED"));
    }

    // Check 3: Level Content
    if (ValidateLevelContent())
    {
        Result.PassedChecks++;
        UE_LOG(LogTemp, Warning, TEXT("✓ Level content validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Level content validation FAILED"));
    }

    // Check 4: QA Integration
    if (ValidateQAIntegration())
    {
        Result.PassedChecks++;
        UE_LOG(LogTemp, Warning, TEXT("✓ QA integration validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ QA integration validation FAILED"));
    }

    // Calculate health percentage
    Result.HealthPercentage = (float(Result.PassedChecks) / float(Result.TotalChecks)) * 100.0f;
    Result.Status = CalculateStatus(Result.HealthPercentage);

    // Store result
    LastValidationResult = Result;

    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION HEALTH: %.1f%% (%d/%d) ==="), 
           Result.HealthPercentage, Result.PassedChecks, Result.TotalChecks);

    return Result;
}

bool UBuild_IntegrationValidator::ValidateCriticalClasses()
{
    int32 LoadedCount = 0;
    LastValidationResult.LoadedClasses.Empty();
    LastValidationResult.FailedClasses.Empty();

    for (const FString& ClassName : CriticalClassNames)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            LoadedCount++;
            LastValidationResult.LoadedClasses.Add(ClassName);
            UE_LOG(LogTemp, Log, TEXT("✓ Class loaded: %s"), *ClassName);
        }
        else
        {
            LastValidationResult.FailedClasses.Add(ClassName);
            UE_LOG(LogTemp, Warning, TEXT("✗ Class failed to load: %s"), *ClassName);
        }
    }

    return LoadedCount >= 5; // At least 5 critical classes must load
}

bool UBuild_IntegrationValidator::ValidateBinaryCompilation()
{
    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesDir = FPaths::Combine(ProjectDir, TEXT("Binaries"));
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    if (!PlatformFile.DirectoryExists(*BinariesDir))
    {
        UE_LOG(LogTemp, Error, TEXT("Binaries directory not found: %s"), *BinariesDir);
        return false;
    }

    // Count binary files
    TArray<FString> BinaryFiles;
    PlatformFile.FindFilesRecursively(BinaryFiles, *BinariesDir, TEXT(".so"));
    PlatformFile.FindFilesRecursively(BinaryFiles, *BinariesDir, TEXT(".dll"));
    PlatformFile.FindFilesRecursively(BinaryFiles, *BinariesDir, TEXT(".dylib"));

    LastValidationResult.BinaryCount = BinaryFiles.Num();
    UE_LOG(LogTemp, Log, TEXT("Found %d binary files"), BinaryFiles.Num());

    return BinaryFiles.Num() >= 1; // At least one binary must exist
}

bool UBuild_IntegrationValidator::ValidateLevelContent()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No world found for level validation"));
        return false;
    }

    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }

    LastValidationResult.ActorCount = ActorCount;
    UE_LOG(LogTemp, Log, TEXT("Level contains %d actors"), ActorCount);

    return ActorCount >= MinimumActorCount;
}

bool UBuild_IntegrationValidator::ValidateQAIntegration()
{
    int32 LoadedQACount = 0;

    for (const FString& QAClassName : QAClassNames)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *QAClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            LoadedQACount++;
            UE_LOG(LogTemp, Log, TEXT("✓ QA Class integrated: %s"), *QAClassName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("✗ QA Class not found: %s"), *QAClassName);
        }
    }

    return LoadedQACount >= 1; // At least one QA class must be integrated
}

EBuild_IntegrationStatus UBuild_IntegrationValidator::GetCurrentIntegrationStatus() const
{
    return LastValidationResult.Status;
}

float UBuild_IntegrationValidator::GetIntegrationHealthPercentage() const
{
    return LastValidationResult.HealthPercentage;
}

EBuild_IntegrationStatus UBuild_IntegrationValidator::CalculateStatus(float HealthPercentage) const
{
    if (HealthPercentage >= HealthyThreshold)
    {
        return EBuild_IntegrationStatus::Healthy;
    }
    else if (HealthPercentage >= ModerateThreshold)
    {
        return EBuild_IntegrationStatus::Moderate;
    }
    else if (HealthPercentage > 0.0f)
    {
        return EBuild_IntegrationStatus::Critical;
    }
    else
    {
        return EBuild_IntegrationStatus::Failed;
    }
}