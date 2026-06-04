#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260604_004");
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    InitializeCycle004Tasks();
    RefreshProductionStatus();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update metrics every 5 seconds
    static float MetricsTimer = 0.0f;
    MetricsTimer += DeltaTime;
    if (MetricsTimer >= 5.0f)
    {
        RefreshProductionStatus();
        MetricsTimer = 0.0f;
    }
}

void AProductionCoordinator::InitializeCycle004Tasks()
{
    AgentTasks.Empty();

    // Critical path for playable prototype
    AssignTaskToAgent(2, TEXT("Engine Architecture validation and UE5.5 compatibility fixes"), TEXT("Core engine headers with compilation fixes"));
    AssignTaskToAgent(3, TEXT("Physics system implementation with collision detection"), TEXT("Working physics components for character and terrain"));
    AssignTaskToAgent(5, TEXT("Procedural terrain generation with height variation"), TEXT("Landscape with hills, valleys, and proper collision"));
    AssignTaskToAgent(9, TEXT("Character controller with WASD movement and camera"), TEXT("Playable character that responds to input"));
    AssignTaskToAgent(10, TEXT("Basic character animations - idle, walk, run, jump"), TEXT("Animation Blueprint with movement states"));
    AssignTaskToAgent(11, TEXT("Dinosaur NPC placement and basic behavior"), TEXT("5+ dinosaur actors with simple AI in the world"));
    AssignTaskToAgent(12, TEXT("Combat system foundation and survival stats"), TEXT("Health/hunger/stamina UI and basic combat"));
    AssignTaskToAgent(15, TEXT("Game narrative foundation and world lore"), TEXT("Game Design Document and story framework"));
    AssignTaskToAgent(18, TEXT("QA testing framework for prototype validation"), TEXT("Automated tests for core gameplay"));
    AssignTaskToAgent(19, TEXT("Build integration and compilation management"), TEXT("Working build pipeline and error tracking"));

    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator: Initialized %d tasks for Cycle 004"), AgentTasks.Num());
}

void AProductionCoordinator::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& ExpectedDeliverable)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.ExpectedDeliverable = ExpectedDeliverable;
    NewTask.bIsCompleted = false;
    NewTask.Priority = 1.0f;

    // Set agent names
    switch (AgentNumber)
    {
    case 2: NewTask.AgentName = TEXT("Engine Architect"); break;
    case 3: NewTask.AgentName = TEXT("Core Systems Programmer"); break;
    case 5: NewTask.AgentName = TEXT("Procedural World Generator"); break;
    case 9: NewTask.AgentName = TEXT("Character Artist"); break;
    case 10: NewTask.AgentName = TEXT("Animation Agent"); break;
    case 11: NewTask.AgentName = TEXT("NPC Behavior Agent"); break;
    case 12: NewTask.AgentName = TEXT("Combat & Enemy AI Agent"); break;
    case 15: NewTask.AgentName = TEXT("Narrative & Dialogue Agent"); break;
    case 18: NewTask.AgentName = TEXT("QA & Testing Agent"); break;
    case 19: NewTask.AgentName = TEXT("Integration & Build Agent"); break;
    default: NewTask.AgentName = FString::Printf(TEXT("Agent_%d"), AgentNumber); break;
    }

    AgentTasks.Add(NewTask);
}

void AProductionCoordinator::MarkTaskCompleted(int32 AgentNumber)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Task completed for Agent %d: %s"), AgentNumber, *Task.AgentName);
            break;
        }
    }
}

FDir_ProductionMetrics AProductionCoordinator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

bool AProductionCoordinator::ValidatePlayablePrototype()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Check for essential components
    bool bHasCharacter = false;
    bool bHasTerrain = false;
    bool bHasLighting = false;
    int32 DinosaurCount = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;

        if (Cast<ACharacter>(Actor))
        {
            bHasCharacter = true;
        }
        else if (Cast<ALandscape>(Actor))
        {
            bHasTerrain = true;
        }
        else if (Cast<ADirectionalLight>(Actor))
        {
            bHasLighting = true;
        }
        else if (Actor->GetName().Contains(TEXT("Dinosaur")) || Actor->GetName().Contains(TEXT("TRex")) || Actor->GetName().Contains(TEXT("Raptor")))
        {
            DinosaurCount++;
        }
    }

    bool bPrototypeReady = bHasCharacter && bHasTerrain && bHasLighting && (DinosaurCount >= 3);
    CurrentMetrics.bPlayablePrototypeReady = bPrototypeReady;

    UE_LOG(LogTemp, Warning, TEXT("Prototype Validation - Character: %s, Terrain: %s, Lighting: %s, Dinosaurs: %d"),
        bHasCharacter ? TEXT("YES") : TEXT("NO"),
        bHasTerrain ? TEXT("YES") : TEXT("NO"),
        bHasLighting ? TEXT("YES") : TEXT("NO"),
        DinosaurCount);

    return bPrototypeReady;
}

void AProductionCoordinator::RefreshProductionStatus()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Count actors in the world
    CurrentMetrics.ActiveActorsInMap = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (*ActorItr)
        {
            CurrentMetrics.ActiveActorsInMap++;
        }
    }

    // Validate prototype status
    ValidatePlayablePrototype();

    // Calculate completion percentage
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bIsCompleted)
        {
            CompletedTasks++;
        }
    }

    float CompletionPercentage = AgentTasks.Num() > 0 ? (float)CompletedTasks / AgentTasks.Num() * 100.0f : 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("Production Status - Actors: %d, Tasks: %d/%d (%.1f%%), Prototype Ready: %s"),
        CurrentMetrics.ActiveActorsInMap,
        CompletedTasks,
        AgentTasks.Num(),
        CompletionPercentage,
        CurrentMetrics.bPlayablePrototypeReady ? TEXT("YES") : TEXT("NO"));
}