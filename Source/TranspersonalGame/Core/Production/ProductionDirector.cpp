#include "ProductionDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AProductionDirector::AProductionDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize production settings
    CurrentPhase = EDir_ProductionPhase::Production;
    ActorCapLimit = 8000.0f;
    LastMetricsUpdate = 0.0f;
    MetricsUpdateInterval = 5.0f; // Update every 5 seconds
    
    // Initialize metrics
    CurrentMetrics = FDir_ProductionMetrics();
    CurrentMetrics.CurrentMilestone = TEXT("Milestone 1 - Walk Around");
}

void AProductionDirector::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default tasks for all agents
    InitializeDefaultTasks();
    
    // Perform initial metrics update
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: System initialized for Cycle 008"));
}

void AProductionDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastMetricsUpdate += DeltaTime;
    
    // Update metrics periodically
    if (LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdateProductionMetrics();
        EnforceActorCap();
        LastMetricsUpdate = 0.0f;
    }
}

void AProductionDirector::AddAgentTask(const FString& AgentName, const FString& TaskDescription, int32 Priority, float EstimatedHours)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.EstimatedHours = EstimatedHours;
    NewTask.bIsCompleted = false;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionDirector: Added task for %s: %s"), *AgentName, *TaskDescription);
}

void AProductionDirector::CompleteTask(const FString& AgentName, const FString& TaskDescription)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName && Task.TaskDescription == TaskDescription)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Task completed - %s: %s"), *AgentName, *TaskDescription);
            break;
        }
    }
    
    UpdateProductionMetrics();
}

TArray<FDir_AgentTask> AProductionDirector::GetTasksForAgent(const FString& AgentName)
{
    TArray<FDir_AgentTask> AgentSpecificTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName && !Task.bIsCompleted)
        {
            AgentSpecificTasks.Add(Task);
        }
    }
    
    return AgentSpecificTasks;
}

int32 AProductionDirector::GetPendingTaskCount()
{
    int32 PendingCount = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bIsCompleted)
        {
            PendingCount++;
        }
    }
    
    return PendingCount;
}

void AProductionDirector::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count all actors in level
    CurrentMetrics.TotalActorsInLevel = 0;
    CurrentMetrics.DinosaurActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            CurrentMetrics.TotalActorsInLevel++;
            
            // Check if it's a dinosaur actor
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
                ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
                ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("dino")))
            {
                CurrentMetrics.DinosaurActorCount++;
            }
        }
    }
    
    // Update task counts
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.PendingTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bIsCompleted)
        {
            CurrentMetrics.CompletedTasks++;
        }
        else
        {
            CurrentMetrics.PendingTasks++;
        }
    }
    
    // Calculate overall progress
    CurrentMetrics.OverallProgress = CalculateOverallProgress();
}

FDir_ProductionMetrics AProductionDirector::GetCurrentMetrics()
{
    return CurrentMetrics;
}

void AProductionDirector::EnforceActorCap()
{
    if (CurrentMetrics.TotalActorsInLevel > ActorCapLimit)
    {
        UE_LOG(LogTemp, Error, TEXT("ProductionDirector: Actor cap exceeded! %d > %f"), 
               CurrentMetrics.TotalActorsInLevel, ActorCapLimit);
        
        CleanupExcessActors();
    }
}

int32 AProductionDirector::CountActorsByType(const FString& ActorType)
{
    if (!GetWorld())
    {
        return 0;
    }
    
    int32 Count = 0;
    FString LowerActorType = ActorType.ToLower();
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(LowerActorType))
            {
                Count++;
            }
        }
    }
    
    return Count;
}

bool AProductionDirector::ValidateMilestone1Requirements()
{
    // Milestone 1: Walk Around requirements
    bool bHasPlayerCharacter = CountActorsByType(TEXT("character")) > 0;
    bool bHasTerrain = CountActorsByType(TEXT("landscape")) > 0;
    bool bHasDinosaurs = CurrentMetrics.DinosaurActorCount >= 3;
    bool bHasLighting = CountActorsByType(TEXT("light")) > 0;
    
    bool bMilestone1Complete = bHasPlayerCharacter && bHasTerrain && bHasDinosaurs && bHasLighting;
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Milestone 1 Validation - Character: %s, Terrain: %s, Dinosaurs: %s, Lighting: %s"),
           bHasPlayerCharacter ? TEXT("OK") : TEXT("MISSING"),
           bHasTerrain ? TEXT("OK") : TEXT("MISSING"),
           bHasDinosaurs ? TEXT("OK") : TEXT("MISSING"),
           bHasLighting ? TEXT("OK") : TEXT("MISSING"));
    
    return bMilestone1Complete;
}

void AProductionDirector::SetCurrentMilestone(const FString& MilestoneName)
{
    CurrentMetrics.CurrentMilestone = MilestoneName;
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Milestone set to %s"), *MilestoneName);
}

float AProductionDirector::CalculateOverallProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    float CompletedWeight = 0.0f;
    float TotalWeight = 0.0f;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        float TaskWeight = FMath::Max(1.0f, Task.EstimatedHours);
        TotalWeight += TaskWeight;
        
        if (Task.bIsCompleted)
        {
            CompletedWeight += TaskWeight;
        }
    }
    
    return TotalWeight > 0.0f ? (CompletedWeight / TotalWeight) * 100.0f : 0.0f;
}

void AProductionDirector::AssignCriticalTasks()
{
    // Clear existing tasks and assign critical Milestone 1 tasks
    AgentTasks.Empty();
    
    // Critical tasks for Milestone 1 - Walk Around
    AddAgentTask(TEXT("Agent #02 Engine Architect"), TEXT("Validate UE5.5 compatibility and core systems"), 10, 2.0f);
    AddAgentTask(TEXT("Agent #03 Core Systems"), TEXT("Implement character movement and physics"), 10, 4.0f);
    AddAgentTask(TEXT("Agent #05 World Generator"), TEXT("Create varied terrain with hills and valleys"), 9, 3.0f);
    AddAgentTask(TEXT("Agent #06 Environment Artist"), TEXT("Place trees, rocks, and natural props"), 8, 2.0f);
    AddAgentTask(TEXT("Agent #08 Lighting"), TEXT("Setup day/night cycle and atmospheric lighting"), 7, 2.0f);
    AddAgentTask(TEXT("Agent #09 Character Artist"), TEXT("Create playable character with animations"), 9, 4.0f);
    AddAgentTask(TEXT("Agent #10 Animation"), TEXT("Implement movement animations and transitions"), 8, 3.0f);
    AddAgentTask(TEXT("Agent #12 Combat AI"), TEXT("Create basic dinosaur AI and behaviors"), 8, 3.0f);
    AddAgentTask(TEXT("Agent #15 Narrative"), TEXT("Write survival game lore and context"), 6, 2.0f);
    AddAgentTask(TEXT("Agent #18 QA"), TEXT("Test basic gameplay and movement"), 7, 2.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Critical tasks assigned for Milestone 1"));
}

TArray<FString> AProductionDirector::GetBlockedAgents()
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bIsCompleted && IsTaskBlocked(Task))
        {
            BlockedAgents.AddUnique(Task.AgentName);
        }
    }
    
    return BlockedAgents;
}

void AProductionDirector::ResolveAgentDependencies()
{
    // Check and resolve dependencies between agents
    ValidateAgentDependencies();
    
    UE_LOG(LogTemp, Log, TEXT("ProductionDirector: Agent dependencies resolved"));
}

void AProductionDirector::GenerateProductionReport()
{
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - CYCLE 008 ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %s"), 
           CurrentPhase == EDir_ProductionPhase::Production ? TEXT("Production") : TEXT("Other"));
    UE_LOG(LogTemp, Warning, TEXT("Current Milestone: %s"), *CurrentMetrics.CurrentMilestone);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d (Cap: %.0f)"), CurrentMetrics.TotalActorsInLevel, ActorCapLimit);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), CurrentMetrics.DinosaurActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), CurrentMetrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Pending Tasks: %d"), CurrentMetrics.PendingTasks);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Valid: %s"), ValidateMilestone1Requirements() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("====================================="));
}

void AProductionDirector::LogCurrentStatus()
{
    GenerateProductionReport();
}

void AProductionDirector::ResetAllTasks()
{
    AgentTasks.Empty();
    InitializeDefaultTasks();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: All tasks reset"));
}

void AProductionDirector::InitializeDefaultTasks()
{
    // Initialize with critical Milestone 1 tasks
    AssignCriticalTasks();
}

void AProductionDirector::ValidateAgentDependencies()
{
    // Check if core systems are ready before allowing other agents to proceed
    bool bCoreSystemsReady = false;
    bool bEngineArchitectReady = false;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName.Contains(TEXT("Engine Architect")) && Task.bIsCompleted)
        {
            bEngineArchitectReady = true;
        }
        if (Task.AgentName.Contains(TEXT("Core Systems")) && Task.bIsCompleted)
        {
            bCoreSystemsReady = true;
        }
    }
    
    // Log dependency status
    UE_LOG(LogTemp, Log, TEXT("ProductionDirector: Engine Architect Ready: %s, Core Systems Ready: %s"),
           bEngineArchitectReady ? TEXT("YES") : TEXT("NO"),
           bCoreSystemsReady ? TEXT("YES") : TEXT("NO"));
}

bool AProductionDirector::IsTaskBlocked(const FDir_AgentTask& Task)
{
    // Simple dependency check - tasks with dependencies on other agents
    if (Task.Dependencies.Len() > 0)
    {
        // Check if dependency tasks are completed
        for (const FDir_AgentTask& OtherTask : AgentTasks)
        {
            if (Task.Dependencies.Contains(OtherTask.AgentName) && !OtherTask.bIsCompleted)
            {
                return true; // Task is blocked
            }
        }
    }
    
    return false; // Task is not blocked
}

void AProductionDirector::CleanupExcessActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Remove excess actors while preserving essential ones
    TArray<AActor*> ActorsToRemove;
    TArray<FString> EssentialTypes = {TEXT("playerstart"), TEXT("directionallight"), TEXT("skylight"), 
                                     TEXT("skyatmosphere"), TEXT("fog"), TEXT("character")};
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName().ToLower();
            bool bIsEssential = false;
            
            for (const FString& EssentialType : EssentialTypes)
            {
                if (ActorName.Contains(EssentialType))
                {
                    bIsEssential = true;
                    break;
                }
            }
            
            if (!bIsEssential)
            {
                ActorsToRemove.Add(Actor);
            }
        }
    }
    
    // Remove excess actors
    int32 ActorsToRemoveCount = FMath::Max(0, CurrentMetrics.TotalActorsInLevel - (int32)ActorCapLimit + 1000);
    for (int32 i = 0; i < FMath::Min(ActorsToRemoveCount, ActorsToRemove.Num()); i++)
    {
        if (ActorsToRemove[i])
        {
            ActorsToRemove[i]->Destroy();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Cleaned up %d excess actors"), 
           FMath::Min(ActorsToRemoveCount, ActorsToRemove.Num()));
}