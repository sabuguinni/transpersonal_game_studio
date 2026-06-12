#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    CycleID = TEXT("PROD_CYCLE_AUTO_20260612_009");
    CurrentCycle = 9;
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgents();
    LogProductionState();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update metrics every 5 seconds
    static float MetricsTimer = 0.0f;
    MetricsTimer += DeltaTime;
    
    if (MetricsTimer >= 5.0f)
    {
        CurrentMetrics = CalculateProductionMetrics();
        MetricsTimer = 0.0f;
    }
}

void AProductionCoordinator::InitializeAgents()
{
    AgentTasks.Empty();
    
    // Initialize all 19 agents with their specializations
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),
        TEXT("Engine Architect"), 
        TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"),
        TEXT("Procedural World Generator"),
        TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"),
        TEXT("Lighting & Atmosphere Agent"),
        TEXT("Character Artist Agent"),
        TEXT("Animation Agent"),
        TEXT("NPC Behavior Agent"),
        TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"),
        TEXT("Quest & Mission Designer"),
        TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"),
        TEXT("VFX Agent"),
        TEXT("QA & Testing Agent"),
        TEXT("Integration & Build Agent")
    };

    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.Status = EDir_AgentStatus::Working;
        NewTask.ProgressPercent = 0.0f;
        NewTask.CycleCount = CurrentCycle;
        
        // Assign specific tasks based on current milestone
        switch (i + 1)
        {
            case 1: // Studio Director
                NewTask.CurrentTask = TEXT("Coordinate playable prototype development");
                break;
            case 2: // Engine Architect
                NewTask.CurrentTask = TEXT("Define core architecture for survival systems");
                break;
            case 3: // Core Systems
                NewTask.CurrentTask = TEXT("Implement physics and collision systems");
                break;
            case 5: // World Generator
                NewTask.CurrentTask = TEXT("Create realistic terrain with height variation");
                break;
            case 9: // Character Artist
                NewTask.CurrentTask = TEXT("Create dinosaur actors with collision");
                break;
            case 10: // Animation
                NewTask.CurrentTask = TEXT("Implement basic dinosaur animations");
                break;
            case 12: // Combat AI
                NewTask.CurrentTask = TEXT("Create survival HUD with health/hunger bars");
                break;
            default:
                NewTask.CurrentTask = FString::Printf(TEXT("Support playable prototype - %s tasks"), *AgentNames[i]);
                break;
        }
        
        AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d agents for Cycle %d"), AgentTasks.Num(), CurrentCycle);
}

void AProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, float Progress)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            Task.ProgressPercent = FMath::Clamp(Progress, 0.0f, 100.0f);
            
            UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s) status updated: %s - %.1f%%"), 
                   AgentID, *Task.AgentName, 
                   *UEnum::GetValueAsString(NewStatus), Progress);
            break;
        }
    }
}

void AProductionCoordinator::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.CurrentTask = TaskDescription;
            Task.Status = EDir_AgentStatus::Working;
            Task.ProgressPercent = 0.0f;
            
            UE_LOG(LogTemp, Warning, TEXT("New task assigned to Agent #%d (%s): %s"), 
                   AgentID, *Task.AgentName, *TaskDescription);
            break;
        }
    }
}

FDir_ProductionMetrics AProductionCoordinator::CalculateProductionMetrics()
{
    FDir_ProductionMetrics Metrics;
    
    // Count actors in level
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        Metrics.TotalActorsInLevel = AllActors.Num();
        
        // Count dinosaurs (actors with "dino" in name)
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("dino"), ESearchCase::IgnoreCase))
            {
                Metrics.DinosaurCount++;
            }
        }
    }
    
    // Calculate agent metrics
    int32 CompletedCount = 0;
    int32 ActiveCount = 0;
    float TotalProgress = 0.0f;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedCount++;
        }
        else if (Task.Status == EDir_AgentStatus::Working)
        {
            ActiveCount++;
        }
        
        TotalProgress += Task.ProgressPercent;
    }
    
    Metrics.CompletedTasks = CompletedCount;
    Metrics.ActiveAgents = ActiveCount;
    Metrics.OverallProgress = AgentTasks.Num() > 0 ? TotalProgress / AgentTasks.Num() : 0.0f;
    
    // Set current milestone and phase
    Metrics.CurrentMilestone = TEXT("Minimum Viable Playable Prototype");
    Metrics.Phase = EDir_ProductionPhase::Production;
    Metrics.BudgetUsed = 52.20f;
    Metrics.BudgetTotal = 100.0f;
    
    return Metrics;
}

void AProductionCoordinator::ValidateMilestone(const FString& MilestoneName)
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Validating milestone '%s'"), *MilestoneName);
    
    if (MilestoneName.Contains(TEXT("Playable")))
    {
        // Check if we have the minimum requirements for playable prototype
        bool bHasCharacter = false;
        bool bHasTerrain = false;
        bool bHasDinosaurs = CurrentMetrics.DinosaurCount > 0;
        
        UWorld* World = GetWorld();
        if (World)
        {
            TArray<AActor*> AllActors;
            UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
            
            for (AActor* Actor : AllActors)
            {
                if (Actor)
                {
                    FString ActorName = Actor->GetName().ToLower();
                    if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
                    {
                        bHasCharacter = true;
                    }
                    if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
                    {
                        bHasTerrain = true;
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Milestone validation - Character: %s, Terrain: %s, Dinosaurs: %s"), 
               bHasCharacter ? TEXT("YES") : TEXT("NO"),
               bHasTerrain ? TEXT("YES") : TEXT("NO"),
               bHasDinosaurs ? TEXT("YES") : TEXT("NO"));
    }
}

TArray<FDir_AgentTask> AProductionCoordinator::GetActiveAgents()
{
    TArray<FDir_AgentTask> ActiveAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            ActiveAgents.Add(Task);
        }
    }
    
    return ActiveAgents;
}

void AProductionCoordinator::LogProductionState()
{
    CurrentMetrics = CalculateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATE CYCLE %d ==="), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), CurrentMetrics.ActiveAgents);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Budget: $%.2f / $%.2f"), CurrentMetrics.BudgetUsed, CurrentMetrics.BudgetTotal);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *UEnum::GetValueAsString(CurrentMetrics.Phase));
    UE_LOG(LogTemp, Warning, TEXT("================================"));
}

void AProductionCoordinator::DebugPrintAgentStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT STATUS DEBUG ==="));
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s): %s - %.1f%% - %s"), 
               Task.AgentID, *Task.AgentName, 
               *UEnum::GetValueAsString(Task.Status),
               Task.ProgressPercent, *Task.CurrentTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

bool AProductionCoordinator::IsAgentBlocked(int32 AgentID)
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            return Task.Status == EDir_AgentStatus::Blocked;
        }
    }
    return false;
}

void AProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentMetrics.Phase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Production phase changed to: %s"), *UEnum::GetValueAsString(NewPhase));
}