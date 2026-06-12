#include "Build_IntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

UBuild_IntegrationManager::UBuild_IntegrationManager()
{
    bIntegrationInProgress = false;
    LastValidationTime = 0.0f;
}

void UBuild_IntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Initialized"));
    
    // Register core modules for validation
    TArray<FString> CoreClasses;
    CoreClasses.Add(TEXT("TranspersonalGameState"));
    CoreClasses.Add(TEXT("TranspersonalCharacter"));
    RegisterModuleForValidation(TEXT("Core"), CoreClasses);
    
    TArray<FString> WorldClasses;
    WorldClasses.Add(TEXT("PCGWorldGenerator"));
    WorldClasses.Add(TEXT("FoliageManager"));
    RegisterModuleForValidation(TEXT("World"), WorldClasses);
    
    TArray<FString> AIClasses;
    AIClasses.Add(TEXT("CrowdSimulationManager"));
    RegisterModuleForValidation(TEXT("AI"), AIClasses);
    
    TArray<FString> QAClasses;
    QAClasses.Add(TEXT("QA_TestFramework"));
    QAClasses.Add(TEXT("QA_PerformanceMonitor"));
    RegisterModuleForValidation(TEXT("QA"), QAClasses);
}

void UBuild_IntegrationManager::Deinitialize()
{
    RegisteredModules.Empty();
    Super::Deinitialize();
}

void UBuild_IntegrationManager::RunFullIntegrationTest()
{
    if (bIntegrationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Integration test already in progress"));
        return;
    }
    
    bIntegrationInProgress = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Starting full integration test"));
    
    // Clear previous report
    LastReport = FBuild_IntegrationReport();
    
    // Enforce actor cap first
    EnforceActorCap(8000);
    
    // Validate all registered modules
    ValidateAllRegisteredModules();
    
    // Check actor counts
    CheckActorCounts();
    
    // Generate final report
    GenerateIntegrationReport();
    
    bIntegrationInProgress = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Integration test completed"));
}

void UBuild_IntegrationManager::EnforceActorCap(int32 MaxActors)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_IntegrationManager: No world found for actor cap enforcement"));
        return;
    }
    
    TArray<AActor*> AllActors;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AllActors.Add(*ActorItr);
    }
    
    LastReport.TotalActorCount = AllActors.Num();
    
    if (AllActors.Num() > MaxActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Actor count %d exceeds cap %d, cleaning up"), AllActors.Num(), MaxActors);
        CleanupExcessActors(MaxActors);
        LastReport.bActorCapEnforced = true;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Actor count %d within cap %d"), AllActors.Num(), MaxActors);
        LastReport.bActorCapEnforced = false;
    }
}

FBuild_IntegrationReport UBuild_IntegrationManager::GetLastIntegrationReport() const
{
    return LastReport;
}

bool UBuild_IntegrationManager::ValidateModuleIntegration(const FString& ModuleName)
{
    if (!RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Module %s not registered"), *ModuleName);
        return false;
    }
    
    const TArray<FString>& RequiredClasses = RegisteredModules[ModuleName];
    int32 LoadedClasses = 0;
    
    for (const FString& ClassName : RequiredClasses)
    {
        if (TestClassLoading(ClassName))
        {
            LoadedClasses++;
        }
    }
    
    bool bModuleValid = (LoadedClasses == RequiredClasses.Num());
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Module %s validation: %d/%d classes loaded"), 
           *ModuleName, LoadedClasses, RequiredClasses.Num());
    
    return bModuleValid;
}

void UBuild_IntegrationManager::RegisterModuleForValidation(const FString& ModuleName, const TArray<FString>& RequiredClasses)
{
    RegisteredModules.Add(ModuleName, RequiredClasses);
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Registered module %s with %d classes"), 
           *ModuleName, RequiredClasses.Num());
}

void UBuild_IntegrationManager::ClearValidationCache()
{
    RegisteredModules.Empty();
    LastReport = FBuild_IntegrationReport();
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Validation cache cleared"));
}

void UBuild_IntegrationManager::ValidateAllRegisteredModules()
{
    LastReport.ModuleStatuses.Empty();
    
    for (const auto& ModulePair : RegisteredModules)
    {
        const FString& ModuleName = ModulePair.Key;
        const TArray<FString>& RequiredClasses = ModulePair.Value;
        
        FBuild_ModuleStatus ModuleStatus;
        ModuleStatus.ModuleName = ModuleName;
        
        float StartTime = FPlatformTime::Seconds();
        
        int32 LoadedClasses = 0;
        for (const FString& ClassName : RequiredClasses)
        {
            if (TestClassLoading(ClassName))
            {
                LoadedClasses++;
            }
        }
        
        ModuleStatus.LoadTime = FPlatformTime::Seconds() - StartTime;
        ModuleStatus.ClassCount = LoadedClasses;
        ModuleStatus.bIsLoaded = (LoadedClasses > 0);
        ModuleStatus.bIsCompiled = (LoadedClasses == RequiredClasses.Num());
        
        LastReport.ModuleStatuses.Add(ModuleStatus);
    }
}

void UBuild_IntegrationManager::CheckActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    LastReport.TotalActorCount = ActorCount;
}

void UBuild_IntegrationManager::GenerateIntegrationReport()
{
    LastReport.TotalBuildTime = FPlatformTime::Seconds() - LastValidationTime;
    
    // Determine overall validation result
    bool bAllModulesValid = true;
    for (const FBuild_ModuleStatus& ModuleStatus : LastReport.ModuleStatuses)
    {
        if (!ModuleStatus.bIsCompiled)
        {
            bAllModulesValid = false;
            break;
        }
    }
    
    if (bAllModulesValid && LastReport.TotalActorCount <= 8000)
    {
        LastReport.ValidationResult = EBuild_ValidationResult::Success;
    }
    else if (LastReport.ModuleStatuses.Num() > 0)
    {
        LastReport.ValidationResult = EBuild_ValidationResult::Warning;
    }
    else
    {
        LastReport.ValidationResult = EBuild_ValidationResult::Failed;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Integration report generated - Result: %d, Modules: %d, Actors: %d"), 
           (int32)LastReport.ValidationResult, LastReport.ModuleStatuses.Num(), LastReport.TotalActorCount);
}

bool UBuild_IntegrationManager::TestClassLoading(const FString& ClassName)
{
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *FullClassName);
    if (LoadedClass)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Build_IntegrationManager: Successfully loaded class %s"), *FullClassName);
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Failed to load class %s"), *FullClassName);
    return false;
}

void UBuild_IntegrationManager::CleanupExcessActors(int32 MaxActors)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    TArray<AActor*> EssentialActors;
    TArray<AActor*> RemovableActors;
    
    // Collect all actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        AllActors.Add(Actor);
        
        FString ActorName = Actor->GetName().ToLower();
        
        // Keep essential actors
        if (ActorName.Contains(TEXT("playerstart")) || 
            ActorName.Contains(TEXT("directionallight")) ||
            ActorName.Contains(TEXT("skylight")) ||
            ActorName.Contains(TEXT("skyatmosphere")) ||
            ActorName.Contains(TEXT("fog")) ||
            ActorName.Contains(TEXT("trex")) ||
            ActorName.Contains(TEXT("veloci")) ||
            ActorName.Contains(TEXT("tricera")) ||
            ActorName.Contains(TEXT("brachi")))
        {
            EssentialActors.Add(Actor);
        }
        else
        {
            RemovableActors.Add(Actor);
        }
    }
    
    // Remove excess actors randomly
    int32 ActorsToRemove = FMath::Max(0, AllActors.Num() - MaxActors);
    if (ActorsToRemove > 0 && RemovableActors.Num() > 0)
    {
        // Shuffle removable actors
        for (int32 i = RemovableActors.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            RemovableActors.Swap(i, j);
        }
        
        // Remove actors
        int32 RemovedCount = 0;
        for (int32 i = 0; i < FMath::Min(ActorsToRemove, RemovableActors.Num()); i++)
        {
            if (RemovableActors[i] && IsValid(RemovableActors[i]))
            {
                RemovableActors[i]->Destroy();
                RemovedCount++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Removed %d excess actors"), RemovedCount);
    }
}