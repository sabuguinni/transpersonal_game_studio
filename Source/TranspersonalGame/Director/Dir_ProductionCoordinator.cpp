#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize production settings
    CurrentPhase = EDir_ProductionPhase::MinimumViablePrototype;
    
    // Initialize critical path flags
    bPlayerCharacterExists = false;
    bTerrainGenerated = false;
    bDinosaursSpawned = false;
    bSurvivalHUDActive = false;
    bBasicLightingSetup = false;

    // Set actor limits from Hugo's directives
    MaxTotalActors = 8000;
    MaxDinosaurs = 150;
    MaxPropsPerBiome = 1000;
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomes();
    InitializeProductionTasks();
    UpdateProductionStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized - Phase: %d"), (int32)CurrentPhase);
}

void ADir_ProductionCoordinator::InitializeBiomes()
{
    BiomeStatuses.Empty();

    // Initialize the 5 biomes from Hugo's specifications
    FDir_BiomeStatus Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0, 0, 0);
    BiomeStatuses.Add(Savana);

    FDir_BiomeStatus Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000, -45000, 0);
    BiomeStatuses.Add(Pantano);

    FDir_BiomeStatus Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000, 40000, 0);
    BiomeStatuses.Add(Floresta);

    FDir_BiomeStatus Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000, 0, 0);
    BiomeStatuses.Add(Deserto);

    FDir_BiomeStatus Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000, 50000, 0);
    BiomeStatuses.Add(Montanha);
}

void ADir_ProductionCoordinator::InitializeProductionTasks()
{
    ActiveTasks.Empty();

    // Critical path tasks for Minimum Viable Prototype
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentName = TEXT("Agent #9 - Character Artist");
    CharacterTask.TaskDescription = TEXT("Ensure TranspersonalCharacter has movement, camera, and survival stats");
    CharacterTask.Priority = 10;
    CharacterTask.EstimatedHours = 2.0f;
    ActiveTasks.Add(CharacterTask);

    FDir_AgentTask TerrainTask;
    TerrainTask.AgentName = TEXT("Agent #5 - World Generator");
    TerrainTask.TaskDescription = TEXT("Generate varied terrain with hills and valleys in all 5 biomes");
    TerrainTask.Priority = 9;
    TerrainTask.EstimatedHours = 3.0f;
    ActiveTasks.Add(TerrainTask);

    FDir_AgentTask DinosaurTask;
    DinosaurTask.AgentName = TEXT("Agent #12 - Combat AI");
    DinosaurTask.TaskDescription = TEXT("Place 150 dinosaurs distributed across biomes with basic AI");
    DinosaurTask.Priority = 8;
    DinosaurTask.EstimatedHours = 4.0f;
    ActiveTasks.Add(DinosaurTask);

    FDir_AgentTask HUDTask;
    HUDTask.AgentName = TEXT("Agent #14 - Quest Designer");
    HUDTask.TaskDescription = TEXT("Implement survival HUD showing health/hunger/thirst/stamina bars");
    HUDTask.Priority = 7;
    HUDTask.EstimatedHours = 2.5f;
    ActiveTasks.Add(HUDTask);

    FDir_AgentTask LightingTask;
    LightingTask.AgentName = TEXT("Agent #8 - Lighting Artist");
    LightingTask.TaskDescription = TEXT("Setup sun, sky atmosphere, and basic fog for all biomes");
    LightingTask.Priority = 6;
    LightingTask.EstimatedHours = 1.5f;
    ActiveTasks.Add(LightingTask);
}

void ADir_ProductionCoordinator::UpdateProductionStatus()
{
    ValidateCriticalPath();
    UpdateBiomeStatuses();
    
    if (IsWithinActorLimits() == false)
    {
        UE_LOG(LogTemp, Error, TEXT("ACTOR LIMIT EXCEEDED - Enforcing limits"));
        EnforceActorLimits();
    }
}

void ADir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDesc, int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDesc;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;
    NewTask.EstimatedHours = 2.0f;
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to %s: %s (Priority: %d)"), *AgentName, *TaskDesc, Priority);
}

void ADir_ProductionCoordinator::CompleteAgentTask(const FString& AgentName)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName && !Task.bIsCompleted)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Task completed by %s: %s"), *AgentName, *Task.TaskDescription);
            break;
        }
    }
}

float ADir_ProductionCoordinator::GetOverallProgress() const
{
    if (ActiveTasks.Num() == 0)
        return 0.0f;

    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.bIsCompleted)
            CompletedTasks++;
    }

    return (float)CompletedTasks / (float)ActiveTasks.Num() * 100.0f;
}

void ADir_ProductionCoordinator::ValidateCriticalPath()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Check for player character
    bPlayerCharacterExists = false;
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && Pawn->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            bPlayerCharacterExists = true;
            break;
        }
    }

    // Check for terrain (landscape actors)
    bTerrainGenerated = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Landscape")))
        {
            bTerrainGenerated = true;
            break;
        }
    }

    // Check for dinosaurs
    bDinosaursSpawned = false;
    int32 DinoCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
                ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")))
            {
                DinoCount++;
            }
        }
    }
    bDinosaursSpawned = (DinoCount > 0);

    // Check for lighting
    bBasicLightingSetup = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetClass()->GetName().Contains(TEXT("DirectionalLight")) ||
                     Actor->GetClass()->GetName().Contains(TEXT("SkyAtmosphere"))))
        {
            bBasicLightingSetup = true;
            break;
        }
    }
}

bool ADir_ProductionCoordinator::IsMinimumViablePrototypeReady() const
{
    return bPlayerCharacterExists && bTerrainGenerated && bDinosaursSpawned && bBasicLightingSetup;
}

void ADir_ProductionCoordinator::UpdateBiomeStatuses()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        Biome.ActorCount = 0;
        Biome.DinosaurCount = 0;

        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (!Actor) continue;

            FVector ActorLocation = Actor->GetActorLocation();
            float Distance = FVector::Dist2D(ActorLocation, Biome.CenterLocation);

            if (Distance <= 20000.0f) // Within biome radius
            {
                Biome.ActorCount++;

                FString ActorName = Actor->GetName().ToLower();
                if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
                    ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")))
                {
                    Biome.DinosaurCount++;
                }
            }
        }

        Biome.bIsOverLimit = (Biome.ActorCount > MaxPropsPerBiome);
    }
}

FVector ADir_ProductionCoordinator::GetOptimalSpawnLocationForBiome(const FString& BiomeName)
{
    FDir_BiomeStatus* Biome = FindBiomeStatus(BiomeName);
    if (!Biome) return FVector::ZeroVector;

    // Return center with slight random offset
    FVector SpawnLocation = Biome->CenterLocation;
    SpawnLocation.X += FMath::RandRange(-5000.0f, 5000.0f);
    SpawnLocation.Y += FMath::RandRange(-5000.0f, 5000.0f);
    SpawnLocation.Z = 100.0f; // Safe height above ground

    return SpawnLocation;
}

bool ADir_ProductionCoordinator::CanSpawnInBiome(const FString& BiomeName) const
{
    const FDir_BiomeStatus* Biome = FindBiomeStatus(BiomeName);
    if (!Biome) return false;

    return !Biome->bIsOverLimit;
}

bool ADir_ProductionCoordinator::IsWithinActorLimits() const
{
    return (GetCurrentActorCount() <= MaxTotalActors) && (GetCurrentDinosaurCount() <= MaxDinosaurs);
}

void ADir_ProductionCoordinator::EnforceActorLimits()
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 CurrentActors = GetCurrentActorCount();
    if (CurrentActors > MaxTotalActors)
    {
        int32 ToDelete = CurrentActors - MaxTotalActors;
        UE_LOG(LogTemp, Error, TEXT("Deleting %d excess actors (Current: %d, Max: %d)"), ToDelete, CurrentActors, MaxTotalActors);

        // Delete oldest non-essential actors first
        TArray<AActor*> ActorsToDelete;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->GetClass()->GetName().Contains(TEXT("PlayerStart")) &&
                !Actor->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
            {
                ActorsToDelete.Add(Actor);
                if (ActorsToDelete.Num() >= ToDelete)
                    break;
            }
        }

        for (AActor* Actor : ActorsToDelete)
        {
            Actor->Destroy();
        }
    }
}

int32 ADir_ProductionCoordinator::GetCurrentActorCount() const
{
    UWorld* World = GetWorld();
    if (!World) return 0;

    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        Count++;
    }
    return Count;
}

int32 ADir_ProductionCoordinator::GetCurrentDinosaurCount() const
{
    UWorld* World = GetWorld();
    if (!World) return 0;

    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
                ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
                ActorName.Contains(TEXT("ankylo")) || ActorName.Contains(TEXT("parasauro")))
            {
                Count++;
            }
        }
    }
    return Count;
}

FDir_BiomeStatus* ADir_ProductionCoordinator::FindBiomeStatus(const FString& BiomeName)
{
    for (FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (Biome.BiomeName == BiomeName)
        {
            return &Biome;
        }
    }
    return nullptr;
}