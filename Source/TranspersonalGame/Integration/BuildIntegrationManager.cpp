#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    CurrentStatus = EBuild_IntegrationStatus::Unknown;
    ValidationInterval = 30.0f; // Validate every 30 seconds

    // Initialize core system class paths
    CoreSystemClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager"),
        TEXT("/Script/TranspersonalGame.ProceduralWorldManager")
    };

    // Initialize QA system class paths
    QASystemClasses = {
        TEXT("/Script/TranspersonalGame.QATestFramework"),
        TEXT("/Script/TranspersonalGame.VFXValidationComponent"),
        TEXT("/Script/TranspersonalGame.AudioTestComponent")
    };
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing integration validation system"));
    
    // Perform initial validation
    FBuild_IntegrationReport InitialReport = ValidateAllSystems();
    
    // Start periodic validation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::PerformPeriodicValidation,
            ValidationInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initialization complete - Status: %d"), (int32)CurrentStatus);
}

void UBuildIntegrationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Deinitializing"));
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuildIntegrationManager::ValidateAllSystems()
{
    FBuild_IntegrationReport Report;
    Report.ValidationTimestamp = FDateTime::Now();
    Report.SystemsTotal = CoreSystemClasses.Num() + QASystemClasses.Num();
    Report.SystemsLoaded = 0;

    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting comprehensive system validation"));

    // Validate core systems
    for (const FString& SystemClass : CoreSystemClasses)
    {
        FBuild_SystemValidationResult Result;
        if (ValidateSystemClass(SystemClass, Result))
        {
            Report.SystemsLoaded++;
        }
        Report.SystemResults.Add(Result);
    }

    // Validate QA systems
    for (const FString& QAClass : QASystemClasses)
    {
        FBuild_SystemValidationResult Result;
        if (ValidateSystemClass(QAClass, Result))
        {
            Report.SystemsLoaded++;
        }
        Report.SystemResults.Add(Result);
    }

    // Calculate integration percentage
    Report.IntegrationPercentage = Report.SystemsTotal > 0 ? 
        (float(Report.SystemsLoaded) / float(Report.SystemsTotal)) * 100.0f : 0.0f;

    // Determine overall status
    if (Report.IntegrationPercentage >= 90.0f)
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Passed;
    }
    else if (Report.IntegrationPercentage >= 70.0f)
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Validating;
    }
    else if (Report.IntegrationPercentage >= 50.0f)
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Failed;
    }
    else
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Critical;
    }

    LastIntegrationReport = Report;
    CurrentStatus = Report.OverallStatus;

    LogIntegrationResults(Report);
    return Report;
}

bool UBuildIntegrationManager::ValidateSystemClass(const FString& ClassName, FBuild_SystemValidationResult& OutResult)
{
    OutResult.SystemName = ClassName;
    OutResult.bIsLoaded = false;
    OutResult.bIsFunctional = false;
    OutResult.ErrorMessage = TEXT("");

    try
    {
        // Attempt to load the class
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        
        if (LoadedClass)
        {
            OutResult.bIsLoaded = true;
            
            // Test basic functionality
            if (TestSystemFunctionality(LoadedClass, OutResult))
            {
                OutResult.bIsFunctional = true;
                UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: System validation PASSED - %s"), *ClassName);
                return true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: System loaded but not functional - %s"), *ClassName);
            }
        }
        else
        {
            OutResult.ErrorMessage = TEXT("Class failed to load");
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Failed to load class - %s"), *ClassName);
        }
    }
    catch (...)
    {
        OutResult.ErrorMessage = TEXT("Exception during class loading");
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Exception loading class - %s"), *ClassName);
    }

    return false;
}

bool UBuildIntegrationManager::TestSystemFunctionality(UClass* SystemClass, FBuild_SystemValidationResult& OutResult)
{
    if (!SystemClass)
    {
        OutResult.ErrorMessage = TEXT("Null system class");
        return false;
    }

    // Basic functionality test - check if class has expected properties/functions
    try
    {
        // Test CDO access
        UObject* CDO = SystemClass->GetDefaultObject();
        if (!CDO)
        {
            OutResult.ErrorMessage = TEXT("Failed to get Class Default Object");
            return false;
        }

        // Test reflection data
        if (SystemClass->GetPropertiesSize() == 0 && SystemClass->NumReplicatedProperties() == 0)
        {
            // Class has no properties - might be a stub
            OutResult.ErrorMessage = TEXT("Class appears to be a stub (no properties)");
            return false;
        }

        return true;
    }
    catch (...)
    {
        OutResult.ErrorMessage = TEXT("Exception during functionality test");
        return false;
    }
}

bool UBuildIntegrationManager::ValidateSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating system dependencies"));
    
    // Check cross-module dependencies
    bool bDependenciesValid = true;

    // Test World Generation <-> Foliage integration
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    UClass* FoliageClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.FoliageManager"));
    
    if (!WorldGenClass || !FoliageClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: World Generation <-> Foliage dependency FAILED"));
        bDependenciesValid = false;
    }

    // Test Character <-> GameState integration
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    UClass* GameStateClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    
    if (!CharacterClass || !GameStateClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Character <-> GameState dependency FAILED"));
        bDependenciesValid = false;
    }

    return bDependenciesValid;
}

bool UBuildIntegrationManager::ValidateMapIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating map integrity"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No world available for map validation"));
        return false;
    }

    // Basic map validation
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }

    if (ActorCount < 10) // Minimum expected actors in MinPlayableMap
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Map has insufficient actors (%d)"), ActorCount);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Map validation passed (%d actors)"), ActorCount);
    return true;
}

FBuild_SystemValidationResult UBuildIntegrationManager::ValidateCoreSystem(const FString& SystemClassName)
{
    FBuild_SystemValidationResult Result;
    ValidateSystemClass(SystemClassName, Result);
    return Result;
}

FBuild_SystemValidationResult UBuildIntegrationManager::ValidateQASystem(const FString& QAClassName)
{
    FBuild_SystemValidationResult Result;
    ValidateSystemClass(QAClassName, Result);
    return Result;
}

bool UBuildIntegrationManager::CreateBuildSnapshot(const FString& SnapshotName)
{
    if (SnapshotName.IsEmpty())
    {
        return false;
    }

    // Create a simple snapshot record
    FString SnapshotData = FString::Printf(TEXT("Snapshot_%s_%s"), 
        *SnapshotName, 
        *FDateTime::Now().ToString());
    
    BuildSnapshots.Add(SnapshotName, SnapshotData);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Created build snapshot - %s"), *SnapshotName);
    return true;
}

bool UBuildIntegrationManager::RestoreBuildSnapshot(const FString& SnapshotName)
{
    if (!BuildSnapshots.Contains(SnapshotName))
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Snapshot not found - %s"), *SnapshotName);
        return false;
    }

    // In a real implementation, this would restore the build state
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Restored build snapshot - %s"), *SnapshotName);
    return true;
}

TArray<FString> UBuildIntegrationManager::GetAvailableSnapshots() const
{
    TArray<FString> SnapshotNames;
    BuildSnapshots.GetKeys(SnapshotNames);
    return SnapshotNames;
}

void UBuildIntegrationManager::PerformPeriodicValidation()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Performing periodic validation"));
    ValidateAllSystems();
}

void UBuildIntegrationManager::LogIntegrationResults(const FBuild_IntegrationReport& Report)
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Status: %d"), (int32)Report.OverallStatus);
    UE_LOG(LogTemp, Warning, TEXT("Systems Loaded: %d/%d (%.1f%%)"), 
        Report.SystemsLoaded, Report.SystemsTotal, Report.IntegrationPercentage);
    
    for (const FBuild_SystemValidationResult& Result : Report.SystemResults)
    {
        if (Result.bIsLoaded && Result.bIsFunctional)
        {
            UE_LOG(LogTemp, Log, TEXT("  ✓ %s"), *Result.SystemName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  ✗ %s - %s"), *Result.SystemName, *Result.ErrorMessage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION REPORT ==="));
}