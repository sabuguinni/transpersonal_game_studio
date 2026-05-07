#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    CurrentBuildStatus = EBuildStatus::Unknown;
    bAutoValidateOnStartup = true;
    bCleanupOrphansOnValidation = true;
    ValidationTimeout = 30.0f;
    TotalActorsInMap = 0;
    ValidatedModules = 0;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing build integration system"));
    
    CurrentBuildStatus = EBuildStatus::Initializing;
    OnBuildStatusChanged.Broadcast(CurrentBuildStatus);
    
    if (bAutoValidateOnStartup)
    {
        // Start validation after a short delay to ensure all systems are loaded
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                ValidationTimerHandle,
                this,
                &UBuildIntegrationManager::StartBuildValidation,
                2.0f,
                false
            );
        }
    }
}

void UBuildIntegrationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    Super::Deinitialize();
}

void UBuildIntegrationManager::StartBuildValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting comprehensive build validation"));
    
    UpdateBuildStatus(EBuildStatus::Validating);
    ValidationErrors.Empty();
    ModuleValidationStatus.Empty();
    
    // Run all validation steps
    ValidateClassLoading();
    ValidateActorIntegrity();
    ValidateSystemDependencies();
    
    // Cleanup if enabled
    if (bCleanupOrphansOnValidation)
    {
        CleanupOrphanedHeaders();
        RemoveDuplicateActors();
    }
    
    // Determine final status
    bool bAllValid = true;
    for (const auto& ModuleStatus : ModuleValidationStatus)
    {
        if (!ModuleStatus.Value)
        {
            bAllValid = false;
            break;
        }
    }
    
    EBuildStatus FinalStatus = bAllValid ? EBuildStatus::Success : EBuildStatus::Failed;
    UpdateBuildStatus(FinalStatus);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation complete. Status: %s"), 
           FinalStatus == EBuildStatus::Success ? TEXT("SUCCESS") : TEXT("FAILED"));
}

void UBuildIntegrationManager::ValidateModule(const FString& ModuleName)
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating module %s"), *ModuleName);
    
    bool bModuleValid = true;
    
    // Try to load a representative class from the module
    FString ClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
    UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
    
    if (!TestClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Failed to load class for module %s"), *ModuleName);
        bModuleValid = false;
        ValidationErrors.Add(FString::Printf(TEXT("Module %s: Class loading failed"), *ModuleName));
    }
    
    ModuleValidationStatus.Add(ModuleName, bModuleValid);
    OnModuleValidated.Broadcast(ModuleName, bModuleValid);
    
    if (bModuleValid)
    {
        ValidatedModules++;
    }
}

bool UBuildIntegrationManager::IsModuleValid(const FString& ModuleName) const
{
    const bool* Status = ModuleValidationStatus.Find(ModuleName);
    return Status ? *Status : false;
}

void UBuildIntegrationManager::RunIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Running integration tests"));
    
    ValidateActorSpawning();
    ValidateSystemInteractions();
}

void UBuildIntegrationManager::ValidateActorSpawning()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Testing actor spawning capabilities"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        ValidationErrors.Add(TEXT("Actor Spawning: No valid world found"));
        return;
    }
    
    // Test spawning basic actors
    TArray<FString> TestClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager")
    };
    
    for (const FString& ClassName : TestClasses)
    {
        FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* ActorClass = LoadClass<AActor>(nullptr, *FullClassName);
        
        if (ActorClass)
        {
            // Try to spawn the actor
            FVector SpawnLocation(0.0f, 0.0f, 100.0f);
            FRotator SpawnRotation = FRotator::ZeroRotator;
            
            AActor* TestActor = World->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation);
            if (TestActor)
            {
                UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Successfully spawned %s"), *ClassName);
                // Clean up test actor
                TestActor->Destroy();
            }
            else
            {
                ValidationErrors.Add(FString::Printf(TEXT("Actor Spawning: Failed to spawn %s"), *ClassName));
            }
        }
        else
        {
            ValidationErrors.Add(FString::Printf(TEXT("Actor Spawning: Failed to load class %s"), *ClassName));
        }
    }
}

void UBuildIntegrationManager::ValidateSystemInteractions()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating system interactions"));
    
    // Test basic system interactions
    UWorld* World = GetWorld();
    if (World)
    {
        TotalActorsInMap = World->GetCurrentLevel()->Actors.Num();
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Found %d actors in current level"), TotalActorsInMap);
    }
}

void UBuildIntegrationManager::CleanupOrphanedHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Cleaning up orphaned headers"));
    
    // This would typically scan the file system for .h files without corresponding .cpp files
    // For now, we'll log the intent and mark it as a cleanup operation
    IdentifyOrphanedFiles();
}

void UBuildIntegrationManager::RemoveDuplicateActors()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Removing duplicate actors"));
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TMap<FString, TArray<AActor*>> ActorsByClass;
    
    // Group actors by class name
    for (AActor* Actor : World->GetCurrentLevel()->Actors)
    {
        if (Actor && IsValid(Actor))
        {
            FString ClassName = Actor->GetClass()->GetName();
            ActorsByClass.FindOrAdd(ClassName).Add(Actor);
        }
    }
    
    // Remove duplicates (keep first, remove others at same location)
    int32 RemovedCount = 0;
    for (auto& ClassGroup : ActorsByClass)
    {
        TArray<AActor*>& Actors = ClassGroup.Value;
        if (Actors.Num() > 1)
        {
            for (int32 i = 1; i < Actors.Num(); i++)
            {
                AActor* Actor = Actors[i];
                // Check if this actor is at the same location as the first one
                if (Actor->GetActorLocation().Equals(Actors[0]->GetActorLocation(), 1.0f))
                {
                    Actor->Destroy();
                    RemovedCount++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Removed %d duplicate actors"), RemovedCount);
}

void UBuildIntegrationManager::OptimizeMapPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Optimizing map performance"));
    
    // Basic performance optimization
    UWorld* World = GetWorld();
    if (World)
    {
        // Force garbage collection
        GEngine->ForceGarbageCollection(true);
        
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Performance optimization complete"));
    }
}

void UBuildIntegrationManager::ValidateClassLoading()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating class loading"));
    
    // Test core module classes
    TArray<FString> CoreModules = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        ValidateModule(ModuleName);
    }
}

void UBuildIntegrationManager::ValidateActorIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating actor integrity"));
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 ValidActors = 0;
    int32 InvalidActors = 0;
    
    for (AActor* Actor : World->GetCurrentLevel()->Actors)
    {
        if (Actor && IsValid(Actor))
        {
            ValidActors++;
        }
        else
        {
            InvalidActors++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Actor integrity - Valid: %d, Invalid: %d"), 
           ValidActors, InvalidActors);
    
    if (InvalidActors > 0)
    {
        ValidationErrors.Add(FString::Printf(TEXT("Actor Integrity: Found %d invalid actors"), InvalidActors));
    }
}

void UBuildIntegrationManager::ValidateSystemDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Validating system dependencies"));
    
    // Check if critical subsystems are available
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: GameInstance subsystem available"));
    }
    else
    {
        ValidationErrors.Add(TEXT("System Dependencies: GameInstance not available"));
    }
}

void UBuildIntegrationManager::UpdateBuildStatus(EBuildStatus NewStatus)
{
    if (CurrentBuildStatus != NewStatus)
    {
        CurrentBuildStatus = NewStatus;
        OnBuildStatusChanged.Broadcast(CurrentBuildStatus);
        
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Build status changed to %d"), (int32)NewStatus);
    }
}

void UBuildIntegrationManager::IdentifyOrphanedFiles()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Identifying orphaned files"));
    
    // This would scan the Source directory for .h files without corresponding .cpp files
    // Implementation would use IFileManager to scan directories
    // For now, we log the operation
}

void UBuildIntegrationManager::RemoveInvalidActors()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Removing invalid actors"));
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> ActorsToRemove;
    
    for (AActor* Actor : World->GetCurrentLevel()->Actors)
    {
        if (Actor && !IsValid(Actor))
        {
            ActorsToRemove.Add(Actor);
        }
    }
    
    for (AActor* Actor : ActorsToRemove)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Removed %d invalid actors"), ActorsToRemove.Num());
}

void UBuildIntegrationManager::ConsolidateDuplicateSystems()
{
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Consolidating duplicate systems"));
    
    // This would identify and consolidate duplicate system implementations
    // For now, we log the operation
}