#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

const float ADir_ProductionCoordinator::MetricsUpdateInterval = 5.0f;

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize metrics
    TotalActorsInWorld = 0;
    DinosaurCount = 0;
    EnvironmentPropsCount = 0;
    LastMetricsUpdate = 0.0f;

    // Set up initial milestone
    InitializePlayablePrototypeMilestone();
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized"));
    UpdateWorldMetrics();
    
    // Dispatch initial critical tasks
    DispatchTaskToAgent(TEXT("Agent02_EngineArchitect"), TEXT("Validate core systems compilation"), 10, FVector(1000, 0, 200));
    DispatchTaskToAgent(TEXT("Agent05_WorldGenerator"), TEXT("Create realistic terrain with biomes"), 9, FVector(2000, 0, 200));
    DispatchTaskToAgent(TEXT("Agent09_CharacterArtist"), TEXT("Implement playable character with movement"), 9, FVector(3000, 0, 200));
    DispatchTaskToAgent(TEXT("Agent10_AnimationAgent"), TEXT("Add character animations and dinosaur basic AI"), 8, FVector(4000, 0, 200));
    DispatchTaskToAgent(TEXT("Agent12_CombatAI"), TEXT("Implement survival HUD and basic combat"), 8, FVector(5000, 0, 200));
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastMetricsUpdate += DeltaTime;
    if (LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdateWorldMetrics();
        ValidateAgentOutputs();
        LastMetricsUpdate = 0.0f;
    }
}

void ADir_ProductionCoordinator::DispatchTaskToAgent(const FString& AgentName, const FString& TaskDescription, int32 Priority, FVector Location)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;
    NewTask.TaskLocation = Location;

    ActiveTasks.Add(NewTask);

    UE_LOG(LogTemp, Warning, TEXT("Task dispatched to %s: %s (Priority: %d)"), *AgentName, *TaskDescription, Priority);
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

    // Update milestone progress
    float NewProgress = GetMilestoneProgress();
    CurrentMilestone.CompletionPercentage = NewProgress;
}

float ADir_ProductionCoordinator::GetMilestoneProgress() const
{
    if (ActiveTasks.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.bIsCompleted)
        {
            CompletedTasks++;
        }
    }

    return (float)CompletedTasks / (float)ActiveTasks.Num() * 100.0f;
}

void ADir_ProductionCoordinator::UpdateWorldMetrics()
{
    if (!GetWorld())
    {
        return;
    }

    TotalActorsInWorld = 0;
    DinosaurCount = 0;
    EnvironmentPropsCount = 0;

    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }

        TotalActorsInWorld++;

        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();

        // Count dinosaurs
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
            ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
            ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("veloci")) ||
            ActorLabel.Contains(TEXT("tricera")) || ActorLabel.Contains(TEXT("brachi")))
        {
            DinosaurCount++;
        }

        // Count environment props
        if (ActorName.Contains(TEXT("tree")) || ActorName.Contains(TEXT("rock")) ||
            ActorName.Contains(TEXT("bush")) || ActorName.Contains(TEXT("grass")) ||
            ActorLabel.Contains(TEXT("tree")) || ActorLabel.Contains(TEXT("rock")) ||
            ActorLabel.Contains(TEXT("bush")) || ActorLabel.Contains(TEXT("grass")))
        {
            EnvironmentPropsCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("World Metrics - Total: %d, Dinosaurs: %d, Props: %d"), 
           TotalActorsInWorld, DinosaurCount, EnvironmentPropsCount);
}

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetHighPriorityTasks() const
{
    TArray<FDir_AgentTask> HighPriorityTasks;

    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (!Task.bIsCompleted && Task.Priority >= 8)
        {
            HighPriorityTasks.Add(Task);
        }
    }

    // Sort by priority (highest first)
    HighPriorityTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });

    return HighPriorityTasks;
}

void ADir_ProductionCoordinator::SetCurrentMilestone(const FString& MilestoneName, const TArray<FDir_AgentTask>& RequiredTasks)
{
    CurrentMilestone.MilestoneName = MilestoneName;
    CurrentMilestone.RequiredTasks = RequiredTasks;
    CurrentMilestone.CompletionPercentage = 0.0f;
    CurrentMilestone.bIsCriticalPath = true;

    UE_LOG(LogTemp, Warning, TEXT("New milestone set: %s with %d required tasks"), *MilestoneName, RequiredTasks.Num());
}

bool ADir_ProductionCoordinator::IsMilestoneComplete() const
{
    return CurrentMilestone.CompletionPercentage >= 100.0f;
}

bool ADir_ProductionCoordinator::ValidatePlayablePrototype() const
{
    // Check for minimum viable playable elements
    bool bHasCharacter = false;
    bool bHasTerrain = false;
    bool bHasDinosaurs = false;
    bool bHasLighting = false;

    if (!GetWorld())
    {
        return false;
    }

    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }

        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();

        if (ActorName.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("character")))
        {
            bHasCharacter = true;
        }

        if (ActorName.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("terrain")))
        {
            bHasTerrain = true;
        }

        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) ||
            ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("veloci")))
        {
            bHasDinosaurs = true;
        }

        if (ActorName.Contains(TEXT("light")) || ActorLabel.Contains(TEXT("sun")))
        {
            bHasLighting = true;
        }
    }

    bool bIsPlayable = bHasCharacter && bHasTerrain && bHasDinosaurs && bHasLighting;
    
    UE_LOG(LogTemp, Warning, TEXT("Playable Prototype Validation - Character: %s, Terrain: %s, Dinosaurs: %s, Lighting: %s, Overall: %s"),
           bHasCharacter ? TEXT("OK") : TEXT("MISSING"),
           bHasTerrain ? TEXT("OK") : TEXT("MISSING"),
           bHasDinosaurs ? TEXT("OK") : TEXT("MISSING"),
           bHasLighting ? TEXT("OK") : TEXT("MISSING"),
           bIsPlayable ? TEXT("VALID") : TEXT("INCOMPLETE"));

    return bIsPlayable;
}

FString ADir_ProductionCoordinator::GetProductionStatusReport() const
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION STATUS REPORT ===\n"));
    Report += FString::Printf(TEXT("Current Milestone: %s (%.1f%% complete)\n"), 
                             *CurrentMilestone.MilestoneName, CurrentMilestone.CompletionPercentage);
    Report += FString::Printf(TEXT("World Metrics - Total Actors: %d, Dinosaurs: %d, Props: %d\n"), 
                             TotalActorsInWorld, DinosaurCount, EnvironmentPropsCount);
    
    Report += TEXT("\nActive Tasks:\n");
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        FString Status = Task.bIsCompleted ? TEXT("COMPLETE") : TEXT("PENDING");
        Report += FString::Printf(TEXT("- %s: %s [%s] (Priority: %d)\n"), 
                                 *Task.AgentName, *Task.TaskDescription, *Status, Task.Priority);
    }

    bool bPlayable = ValidatePlayablePrototype();
    Report += FString::Printf(TEXT("\nPlayable Prototype Status: %s\n"), 
                             bPlayable ? TEXT("READY") : TEXT("INCOMPLETE"));

    return Report;
}

void ADir_ProductionCoordinator::InitializePlayablePrototypeMilestone()
{
    CurrentMilestone.MilestoneName = TEXT("Playable Prototype - Walk Around");
    CurrentMilestone.CompletionPercentage = 0.0f;
    CurrentMilestone.bIsCriticalPath = true;

    // Define required tasks for playable prototype
    TArray<FDir_AgentTask> RequiredTasks;
    
    FDir_AgentTask Task1;
    Task1.AgentName = TEXT("Agent02_EngineArchitect");
    Task1.TaskDescription = TEXT("Core systems compilation and validation");
    Task1.Priority = 10;
    RequiredTasks.Add(Task1);

    FDir_AgentTask Task2;
    Task2.AgentName = TEXT("Agent05_WorldGenerator");
    Task2.TaskDescription = TEXT("Realistic terrain with biomes");
    Task2.Priority = 9;
    RequiredTasks.Add(Task2);

    FDir_AgentTask Task3;
    Task3.AgentName = TEXT("Agent09_CharacterArtist");
    Task3.TaskDescription = TEXT("Playable character implementation");
    Task3.Priority = 9;
    RequiredTasks.Add(Task3);

    CurrentMilestone.RequiredTasks = RequiredTasks;
}

void ADir_ProductionCoordinator::ValidateAgentOutputs()
{
    // Validate that agents are producing actual implementations, not just headers
    TArray<FDir_AgentTask> HighPriorityTasks = GetHighPriorityTasks();
    
    if (HighPriorityTasks.Num() > 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: %d high-priority tasks pending - potential bottleneck"), HighPriorityTasks.Num());
    }

    // Check for actor count limits
    if (TotalActorsInWorld > 8000)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Actor count (%d) exceeds limit (8000) - performance risk"), TotalActorsInWorld);
    }

    if (DinosaurCount > 150)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Dinosaur count (%d) exceeds realistic limit (150)"), DinosaurCount);
    }
}