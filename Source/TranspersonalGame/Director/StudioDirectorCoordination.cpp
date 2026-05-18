#include "StudioDirectorCoordination.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UStudioDirectorCoordination::UStudioDirectorCoordination()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260518_004");
    TotalActorsInMap = 0;
    bMinPlayableMapReady = false;
}

void UStudioDirectorCoordination::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCoordination();
    ScanCurrentMapStatus();
}

void UStudioDirectorCoordination::InitializeCoordination()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing coordination system"));
    
    SetupDefaultBiomes();
    
    // Setup initial agent tasks for this cycle
    AssignTaskToAgent(2, TEXT("Review compilation status and fix missing .cpp files"), TEXT("High"));
    AssignTaskToAgent(3, TEXT("Implement collision for spawned dinosaurs"), TEXT("High"));
    AssignTaskToAgent(5, TEXT("Expand terrain to 10km2 with proper biome separation"), TEXT("Critical"));
    AssignTaskToAgent(6, TEXT("Add vegetation and props to populated biomes"), TEXT("Medium"));
    AssignTaskToAgent(9, TEXT("Ensure TranspersonalCharacter has proper animations"), TEXT("Medium"));
    AssignTaskToAgent(12, TEXT("Add basic dinosaur behavior and AI"), TEXT("High"));
    
    LogCoordinationStatus();
}

void UStudioDirectorCoordination::SetupDefaultBiomes()
{
    BiomeStatuses.Empty();
    
    // Setup the 5 core biomes with their coordinates
    FDir_BiomeStatus Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.Location = FVector(0, 0, 100);
    BiomeStatuses.Add(Savana);
    
    FDir_BiomeStatus Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.Location = FVector(-50000, -45000, 100);
    BiomeStatuses.Add(Pantano);
    
    FDir_BiomeStatus Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.Location = FVector(-45000, 40000, 100);
    BiomeStatuses.Add(Floresta);
    
    FDir_BiomeStatus Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.Location = FVector(55000, 0, 100);
    BiomeStatuses.Add(Deserto);
    
    FDir_BiomeStatus Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.Location = FVector(40000, 50000, 100);
    BiomeStatuses.Add(Montanha);
}

void UStudioDirectorCoordination::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bCompleted = false;
    NewTask.CycleID = CurrentCycleID;
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Assigned task to Agent #%d: %s [%s]"), 
           AgentNumber, *TaskDescription, *Priority);
}

void UStudioDirectorCoordination::CompleteAgentTask(int32 AgentNumber, const FString& CycleID)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber && Task.CycleID == CycleID)
        {
            Task.bCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent #%d completed task: %s"), 
                   AgentNumber, *Task.TaskDescription);
            break;
        }
    }
}

void UStudioDirectorCoordination::UpdateBiomeStatus(const FString& BiomeName, const FVector& Location, int32 ActorCount)
{
    for (FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (Biome.BiomeName == BiomeName)
        {
            Biome.ActorCount = ActorCount;
            Biome.bPopulated = (ActorCount >= 500); // Criterion from memory
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Updated %s biome - %d actors"), 
                   *BiomeName, ActorCount);
            return;
        }
    }
}

void UStudioDirectorCoordination::ScanCurrentMapStatus()
{
    if (!GetWorld())
    {
        return;
    }
    
    TotalActorsInMap = 0;
    
    // Count all actors in the world
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && IsValid(Actor))
        {
            TotalActorsInMap++;
        }
    }
    
    // Update biome populations based on proximity
    for (FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        int32 BiomeActorCount = 0;
        
        for (TActorIterator<AStaticMeshActor> MeshIterator(GetWorld()); MeshIterator; ++MeshIterator)
        {
            AStaticMeshActor* MeshActor = *MeshIterator;
            if (MeshActor && IsValid(MeshActor))
            {
                float Distance = FVector::Dist(MeshActor->GetActorLocation(), Biome.Location);
                if (Distance <= 5000.0f) // Within 5km of biome center
                {
                    BiomeActorCount++;
                }
            }
        }
        
        Biome.ActorCount = BiomeActorCount;
        Biome.bPopulated = (BiomeActorCount >= 500);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Scanned map - %d total actors"), TotalActorsInMap);
}

bool UStudioDirectorCoordination::ValidateMinPlayablePrototype()
{
    // Check if we have the minimum viable playable prototype
    bool bHasCharacter = false;
    bool bHasTerrain = false;
    bool bHasDinosaurs = false;
    
    if (!GetWorld())
    {
        return false;
    }
    
    // Check for TranspersonalCharacter
    for (TActorIterator<APawn> PawnIterator(GetWorld()); PawnIterator; ++PawnIterator)
    {
        APawn* Pawn = *PawnIterator;
        if (Pawn && Pawn->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            bHasCharacter = true;
            break;
        }
    }
    
    // Check for terrain (landscape or static mesh terrain)
    bHasTerrain = (TotalActorsInMap > 50); // Basic check for populated world
    
    // Check for dinosaur actors
    for (TActorIterator<AStaticMeshActor> MeshIterator(GetWorld()); MeshIterator; ++MeshIterator)
    {
        AStaticMeshActor* MeshActor = *MeshIterator;
        if (MeshActor && MeshActor->GetActorLabel().Contains(TEXT("rex")) || 
            MeshActor->GetActorLabel().Contains(TEXT("raptor")) ||
            MeshActor->GetActorLabel().Contains(TEXT("ceratops")))
        {
            bHasDinosaurs = true;
            break;
        }
    }
    
    bMinPlayableMapReady = (bHasCharacter && bHasTerrain && bHasDinosaurs);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Prototype validation - Character:%s Terrain:%s Dinosaurs:%s"), 
           bHasCharacter ? TEXT("YES") : TEXT("NO"),
           bHasTerrain ? TEXT("YES") : TEXT("NO"),
           bHasDinosaurs ? TEXT("YES") : TEXT("NO"));
    
    return bMinPlayableMapReady;
}

FString UStudioDirectorCoordination::GenerateCoordinationReport()
{
    FString Report = TEXT("=== STUDIO DIRECTOR COORDINATION REPORT ===\n");
    Report += FString::Printf(TEXT("Cycle ID: %s\n"), *CurrentCycleID);
    Report += FString::Printf(TEXT("Total Actors in Map: %d\n"), TotalActorsInMap);
    Report += FString::Printf(TEXT("Min Playable Prototype Ready: %s\n\n"), 
                             bMinPlayableMapReady ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("BIOME STATUS:\n");
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        Report += FString::Printf(TEXT("  %s: %d actors [%s]\n"), 
                                 *Biome.BiomeName, 
                                 Biome.ActorCount,
                                 Biome.bPopulated ? TEXT("POPULATED") : TEXT("NEEDS WORK"));
    }
    
    Report += TEXT("\nACTIVE TASKS:\n");
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        Report += FString::Printf(TEXT("  Agent #%d: %s [%s] %s\n"), 
                                 Task.AgentNumber,
                                 *Task.TaskDescription,
                                 *Task.Priority,
                                 Task.bCompleted ? TEXT("COMPLETED") : TEXT("PENDING"));
    }
    
    return Report;
}

void UStudioDirectorCoordination::SpawnDinosaurInBiome(const FString& BiomeName, const FString& DinosaurType)
{
    // This function would be called by UE5 Python scripts
    // Implementation would load the appropriate dinosaur asset and spawn it
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Request to spawn %s in %s biome"), 
           *DinosaurType, *BiomeName);
}

void UStudioDirectorCoordination::ValidateBiomePopulation()
{
    ScanCurrentMapStatus();
    
    for (const FDir_BiomeStatus& Biome : BiomeStatuses)
    {
        if (!Biome.bPopulated)
        {
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Biome %s needs population - only %d actors"), 
                   *Biome.BiomeName, Biome.ActorCount);
        }
    }
}

void UStudioDirectorCoordination::CheckAgentProgress()
{
    int32 CompletedTasks = 0;
    int32 PendingTasks = 0;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.bCompleted)
        {
            CompletedTasks++;
        }
        else
        {
            PendingTasks++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Task progress - %d completed, %d pending"), 
           CompletedTasks, PendingTasks);
}

void UStudioDirectorCoordination::LogCoordinationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Coordination system initialized for cycle %s"), 
           *CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Tracking %d biomes and %d active tasks"), 
           BiomeStatuses.Num(), ActiveTasks.Num());
}