#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/Level.h"
#include "EngineUtils.h"

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing integration system"));
    
    InitializeSystemModules();
    RefreshSystemStatus();
    
    // Set initial validation time
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Integration system initialized successfully"));
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Shutting down integration system"));
    
    SystemModules.Empty();
    CurrentStatus = FBuild_IntegrationStatus();
    LastValidationResult = FBuild_ValidationResult();
    
    Super::Deinitialize();
}

void UBuildIntegrationManager::InitializeSystemModules()
{
    SystemModules.Empty();
    
    // Core game modules
    SystemModules.Add(TEXT("TranspersonalGame"));
    SystemModules.Add(TEXT("Engine"));
    SystemModules.Add(TEXT("Core"));
    SystemModules.Add(TEXT("CoreUObject"));
    
    // Game-specific modules
    SystemModules.Add(TEXT("PCGWorldGenerator"));
    SystemModules.Add(TEXT("FoliageManager"));
    SystemModules.Add(TEXT("CrowdSimulation"));
    SystemModules.Add(TEXT("ProceduralWorld"));
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initialized %d system modules"), SystemModules.Num());
}

FBuild_IntegrationStatus UBuildIntegrationManager::GetIntegrationStatus() const
{
    return CurrentStatus;
}

bool UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting full system validation"));
    
    bool bAllValid = true;
    int32 ValidModules = 0;
    
    CurrentStatus.LoadedModules.Empty();
    CurrentStatus.FailedModules.Empty();
    
    for (const FString& ModuleName : SystemModules)
    {
        if (ValidateModule(ModuleName))
        {
            CurrentStatus.LoadedModules.Add(ModuleName);
            ValidModules++;
        }
        else
        {
            CurrentStatus.FailedModules.Add(ModuleName);
            bAllValid = false;
        }
    }
    
    CurrentStatus.bAllModulesLoaded = bAllValid;
    CurrentStatus.bIsCompiled = bAllValid;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation complete - %d/%d modules valid"), 
           ValidModules, SystemModules.Num());
    
    return bAllValid;
}

bool UBuildIntegrationManager::ValidateModule(const FString& ModuleName)
{
    // Check if module is loaded
    if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Module %s is loaded"), *ModuleName);
        return true;
    }
    
    // Try to load module if not loaded
    if (FModuleManager::Get().ModuleExists(*ModuleName))
    {
        try
        {
            FModuleManager::Get().LoadModule(*ModuleName);
            UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Successfully loaded module %s"), *ModuleName);
            return true;
        }
        catch (...)
        {
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Failed to load module %s"), *ModuleName);
            return false;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Module %s does not exist"), *ModuleName);
    return false;
}

FBuild_ValidationResult UBuildIntegrationManager::RunFullValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Running comprehensive validation"));
    
    FBuild_ValidationResult Result;
    float StartTime = FPlatformTime::Seconds();
    
    Result.ErrorMessages.Empty();
    Result.WarningMessages.Empty();
    Result.ErrorCount = 0;
    Result.WarningCount = 0;
    
    // Validate modules
    if (!ValidateAllSystems())
    {
        Result.ErrorCount++;
        Result.ErrorMessages.Add(TEXT("Module validation failed"));
    }
    
    // Validate actors
    if (!ValidateActorIntegrity())
    {
        Result.WarningCount++;
        Result.WarningMessages.Add(TEXT("Some actors have integrity issues"));
    }
    
    // Update actor count
    CurrentStatus.ActiveActorCount = GetActiveActorCount();
    
    // Set build time
    CurrentStatus.LastBuildTime = FDateTime::Now().ToString();
    
    Result.ValidationDuration = FPlatformTime::Seconds() - StartTime;
    Result.bValidationPassed = (Result.ErrorCount == 0);
    
    LastValidationResult = Result;
    LastValidationTime = FPlatformTime::Seconds();
    
    LogValidationResults(Result);
    
    return Result;
}

bool UBuildIntegrationManager::CheckModuleCompatibility(const FString& ModuleName)
{
    if (!FModuleManager::Get().IsModuleLoaded(*ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Module %s not loaded for compatibility check"), *ModuleName);
        return false;
    }
    
    // Basic compatibility check - module is loaded and functional
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Module %s compatibility check passed"), *ModuleName);
    return true;
}

void UBuildIntegrationManager::RefreshSystemStatus()
{
    UpdateIntegrationStatus();
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: System status refreshed - %d actors, %d loaded modules"), 
           CurrentStatus.ActiveActorCount, CurrentStatus.LoadedModules.Num());
}

bool UBuildIntegrationManager::TriggerBuildValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Triggering build validation"));
    
    FBuild_ValidationResult Result = RunFullValidation();
    return Result.bValidationPassed;
}

TArray<FString> UBuildIntegrationManager::GetLoadedModuleList() const
{
    return CurrentStatus.LoadedModules;
}

TArray<FString> UBuildIntegrationManager::GetFailedModuleList() const
{
    return CurrentStatus.FailedModules;
}

int32 UBuildIntegrationManager::GetActiveActorCount() const
{
    if (UWorld* World = GetWorld())
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ActorItr->IsValidLowLevel() && !ActorItr->IsPendingKill())
            {
                ActorCount++;
            }
        }
        return ActorCount;
    }
    
    return 0;
}

bool UBuildIntegrationManager::ValidateActorIntegrity()
{
    if (UWorld* World = GetWorld())
    {
        int32 ValidActors = 0;
        int32 InvalidActors = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->IsValidLowLevel() && !Actor->IsPendingKill())
            {
                ValidActors++;
            }
            else
            {
                InvalidActors++;
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Actor integrity check - %d valid, %d invalid"), 
               ValidActors, InvalidActors);
        
        return (InvalidActors == 0);
    }
    
    return false;
}

void UBuildIntegrationManager::CleanupInvalidActors()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> ActorsToDestroy;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && (Actor->IsPendingKill() || !Actor->IsValidLowLevel()))
            {
                ActorsToDestroy.Add(Actor);
            }
        }
        
        for (AActor* Actor : ActorsToDestroy)
        {
            if (Actor && Actor->IsValidLowLevel())
            {
                Actor->Destroy();
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Cleaned up %d invalid actors"), ActorsToDestroy.Num());
    }
}

void UBuildIntegrationManager::UpdateIntegrationStatus()
{
    CurrentStatus.ActiveActorCount = GetActiveActorCount();
    CurrentStatus.LastBuildTime = FDateTime::Now().ToString();
    
    // Update module status
    ValidateAllSystems();
}

void UBuildIntegrationManager::LogValidationResults(const FBuild_ValidationResult& Result)
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION VALIDATION RESULTS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Validation Passed: %s"), Result.bValidationPassed ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Errors: %d, Warnings: %d"), Result.ErrorCount, Result.WarningCount);
    UE_LOG(LogTemp, Warning, TEXT("Duration: %.2f seconds"), Result.ValidationDuration);
    
    for (const FString& Error : Result.ErrorMessages)
    {
        UE_LOG(LogTemp, Error, TEXT("ERROR: %s"), *Error);
    }
    
    for (const FString& Warning : Result.WarningMessages)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: %s"), *Warning);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END VALIDATION RESULTS ==="));
}