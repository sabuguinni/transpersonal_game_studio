#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Level.h"
#include "EngineUtils.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    InitializeCoreSystemClasses();
}

void UBuild_IntegrationValidator::InitializeCoreSystemClasses()
{
    CoreSystemClasses.Empty();
    CoreSystemClasses.Add(TEXT("TranspersonalGameState"));
    CoreSystemClasses.Add(TEXT("TranspersonalCharacter"));
    CoreSystemClasses.Add(TEXT("PCGWorldGenerator"));
    CoreSystemClasses.Add(TEXT("FoliageManager"));
    CoreSystemClasses.Add(TEXT("CrowdSimulationManager"));
    CoreSystemClasses.Add(TEXT("ProceduralWorldManager"));
    CoreSystemClasses.Add(TEXT("BuildIntegrationManager"));
    CoreSystemClasses.Add(TEXT("QA_TestFramework"));
}

FBuild_ValidationResult UBuild_IntegrationValidator::ValidateProjectIntegration()
{
    FBuild_ValidationResult Result;
    Result.ReportTimestamp = FDateTime::Now().ToString();

    // Validate class loading
    TArray<FString> FailedClasses;
    bool ClassValidation = ValidateClassLoading(CoreSystemClasses, FailedClasses);
    Result.LoadedClasses = CoreSystemClasses.Num() - FailedClasses.Num();
    Result.TotalClasses = CoreSystemClasses.Num();
    Result.FailedClasses = FailedClasses;

    // Count compiled binaries
    Result.BinaryFiles = CountCompiledBinaries();

    // Count level actors
    Result.LevelActors = CountLevelActors();

    // Calculate health percentage
    int32 PassedChecks = 0;
    int32 TotalChecks = 4;

    // Check 1: Class loading (80% threshold)
    if (Result.LoadedClasses >= Result.TotalClasses * 0.8f)
    {
        PassedChecks++;
    }

    // Check 2: Binary compilation
    if (Result.BinaryFiles > 0)
    {
        PassedChecks++;
    }

    // Check 3: Level population (20+ actors)
    if (Result.LevelActors >= 20)
    {
        PassedChecks++;
    }

    // Check 4: Core systems presence
    TArray<FString> CoreSystems = {TEXT("TranspersonalCharacter"), TEXT("TranspersonalGameState"), TEXT("PCGWorldGenerator")};
    int32 CoreLoaded = 0;
    for (const FString& CoreClass : CoreSystems)
    {
        if (!FailedClasses.Contains(CoreClass))
        {
            CoreLoaded++;
        }
    }
    if (CoreLoaded == CoreSystems.Num())
    {
        PassedChecks++;
    }

    Result.HealthPercentage = (float(PassedChecks) / float(TotalChecks)) * 100.0f;
    Result.Status = CalculateIntegrationStatus(Result.HealthPercentage);

    LastValidationResult = Result;
    LogValidationResults(Result);
    GenerateIntegrationReport(Result);

    return Result;
}

bool UBuild_IntegrationValidator::ValidateClassLoading(const TArray<FString>& ClassNames, TArray<FString>& FailedClasses)
{
    FailedClasses.Empty();
    int32 LoadedCount = 0;

    for (const FString& ClassName : ClassNames)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            LoadedCount++;
            UE_LOG(LogTemp, Log, TEXT("✓ Class loaded: %s"), *ClassName);
        }
        else
        {
            FailedClasses.Add(ClassName);
            UE_LOG(LogTemp, Warning, TEXT("✗ Class failed to load: %s"), *ClassName);
        }
    }

    return FailedClasses.Num() == 0;
}

int32 UBuild_IntegrationValidator::CountCompiledBinaries()
{
    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesDir = FPaths::Combine(ProjectDir, TEXT("Binaries"));
    
    int32 BinaryCount = 0;
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    if (PlatformFile.DirectoryExists(*BinariesDir))
    {
        TArray<FString> FoundFiles;
        PlatformFile.FindFilesRecursively(FoundFiles, *BinariesDir, TEXT(".so"));
        PlatformFile.FindFilesRecursively(FoundFiles, *BinariesDir, TEXT(".dll"));
        PlatformFile.FindFilesRecursively(FoundFiles, *BinariesDir, TEXT(".dylib"));
        
        BinaryCount = FoundFiles.Num();
        UE_LOG(LogTemp, Log, TEXT("Found %d compiled binary files"), BinaryCount);
    }
    
    return BinaryCount;
}

int32 UBuild_IntegrationValidator::CountLevelActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }

    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsPendingKill())
        {
            ActorCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Total actors in level: %d"), ActorCount);
    return ActorCount;
}

EBuild_IntegrationStatus UBuild_IntegrationValidator::CalculateIntegrationStatus(float HealthPercentage)
{
    if (HealthPercentage >= HealthyThreshold)
    {
        return EBuild_IntegrationStatus::Healthy;
    }
    else if (HealthPercentage >= StableThreshold)
    {
        return EBuild_IntegrationStatus::Stable;
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

void UBuild_IntegrationValidator::GenerateIntegrationReport(const FBuild_ValidationResult& Result)
{
    FString ReportContent = FString::Printf(
        TEXT("INTEGRATION VALIDATION REPORT\n")
        TEXT("Timestamp: %s\n")
        TEXT("Integration Health: %.0f%%\n")
        TEXT("Status: %s\n")
        TEXT("Loaded Classes: %d/%d\n")
        TEXT("Binary Files: %d\n")
        TEXT("Level Actors: %d\n"),
        *Result.ReportTimestamp,
        Result.HealthPercentage,
        *UEnum::GetValueAsString(Result.Status),
        Result.LoadedClasses,
        Result.TotalClasses,
        Result.BinaryFiles,
        Result.LevelActors
    );

    if (Result.FailedClasses.Num() > 0)
    {
        ReportContent += TEXT("Failed Classes:\n");
        for (const FString& FailedClass : Result.FailedClasses)
        {
            ReportContent += FString::Printf(TEXT("  - %s\n"), *FailedClass);
        }
    }

    FString ReportPath = FPaths::Combine(FPaths::ProjectLogDir(), TEXT("IntegrationReport.txt"));
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Log, TEXT("Integration report saved to: %s"), *ReportPath);
}

void UBuild_IntegrationValidator::LogValidationResults(const FBuild_ValidationResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("=== INTEGRATION VALIDATION RESULTS ==="));
    UE_LOG(LogTemp, Log, TEXT("Health: %.0f%% (%s)"), Result.HealthPercentage, *UEnum::GetValueAsString(Result.Status));
    UE_LOG(LogTemp, Log, TEXT("Classes: %d/%d loaded"), Result.LoadedClasses, Result.TotalClasses);
    UE_LOG(LogTemp, Log, TEXT("Binaries: %d files"), Result.BinaryFiles);
    UE_LOG(LogTemp, Log, TEXT("Actors: %d in level"), Result.LevelActors);

    if (Result.Status == EBuild_IntegrationStatus::Healthy)
    {
        UE_LOG(LogTemp, Log, TEXT("🟢 BUILD STATUS: HEALTHY - Ready for next development cycle"));
    }
    else if (Result.Status == EBuild_IntegrationStatus::Stable)
    {
        UE_LOG(LogTemp, Warning, TEXT("🟡 BUILD STATUS: STABLE - Minor issues need attention"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("🔴 BUILD STATUS: CRITICAL - Major integration problems"));
    }
}

bool UBuild_IntegrationValidator::CheckBinaryCompilation()
{
    return CountCompiledBinaries() > 0;
}