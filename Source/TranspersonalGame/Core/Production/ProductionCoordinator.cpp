#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentPhase = EDir_ProductionPhase::PrototypePhase;
    CycleNumber = 2;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260613_002");
    
    // Initialize default metrics
    Metrics.TotalTasks = 0;
    Metrics.CompletedTasks = 0;
    Metrics.BlockedTasks = 0;
    Metrics.OverallProgress = 0.0f;
    Metrics.ActiveAgents = 19; // 19 agent chain
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized for cycle: %s"), *CurrentCycleID);
    
    // Initialize agent tasks for current cycle
    InitializeProductionCycle(CurrentCycleID);
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update metrics every frame
    UpdateMetrics();
}

void ADir_ProductionCoordinator::InitializeProductionCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    AgentTasks.Empty();
    
    // Define tasks for current prototype phase
    TArray<FString> CoreTasks = {
        TEXT("Engine Architect: Define core systems architecture"),
        TEXT("Core Systems: Implement physics and collision"),
        TEXT("World Generator: Create procedural terrain"),
        TEXT("Environment Artist: Populate world with assets"),
        TEXT("Character Artist: Create player character"),
        TEXT("Animation Agent: Implement character movement"),
        TEXT("Combat AI: Basic dinosaur AI behavior"),
        TEXT("Quest Designer: Create survival objectives"),
        TEXT("Audio Agent: Implement ambient sounds"),
        TEXT("VFX Agent: Add particle effects"),
        TEXT("QA Testing: Validate playable prototype"),
        TEXT("Integration: Build final prototype")
    };
    
    for (int32 i = 0; i < CoreTasks.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.TaskDescription = CoreTasks[i];
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = i + 1;
        NewTask.EstimatedHours = 2.0f;
        
        // Extract agent name
        FString Left, Right;
        if (CoreTasks[i].Split(TEXT(":"), &Left, &Right))
        {
            NewTask.AgentName = Left.TrimStartAndEnd();
        }
        
        AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d tasks for cycle %s"), AgentTasks.Num(), *CycleID);
}

void ADir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDesc, int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDesc;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.Priority = Priority;
    NewTask.EstimatedHours = 2.0f;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Assigned task to %s: %s"), *AgentName, *TaskDesc);
}

void ADir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Log, TEXT("Updated %s status to %d"), *AgentName, (int32)NewStatus);
            break;
        }
    }
}

FDir_ProductionMetrics ADir_ProductionCoordinator::CalculateMetrics()
{
    UpdateMetrics();
    return Metrics;
}

bool ADir_ProductionCoordinator::CanAgentProceed(const FString& AgentName)
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            return CheckDependencies(Task);
        }
    }
    return true; // If no specific task, agent can proceed
}

TArray<FString> ADir_ProductionCoordinator::GetBlockedAgents()
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(Task.AgentName);
        }
    }
    
    return BlockedAgents;
}

void ADir_ProductionCoordinator::AdvanceProductionPhase()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::PrototypePhase;
            break;
        case EDir_ProductionPhase::PrototypePhase:
            CurrentPhase = EDir_ProductionPhase::ProductionPhase;
            break;
        case EDir_ProductionPhase::ProductionPhase:
            CurrentPhase = EDir_ProductionPhase::PolishPhase;
            break;
        case EDir_ProductionPhase::PolishPhase:
            CurrentPhase = EDir_ProductionPhase::ReleaseCandidate;
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced to production phase: %d"), (int32)CurrentPhase);
}

void ADir_ProductionCoordinator::ValidateProductionState()
{
    UpdateMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATE VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s"), *CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Total Tasks: %d"), Metrics.TotalTasks);
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d"), Metrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked: %d"), Metrics.BlockedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Progress: %.1f%%"), Metrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), Metrics.ActiveAgents);
    
    // Log blocked agents
    TArray<FString> Blocked = GetBlockedAgents();
    if (Blocked.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BLOCKED AGENTS: %s"), *FString::Join(Blocked, TEXT(", ")));
    }
}

void ADir_ProductionCoordinator::UpdateMetrics()
{
    Metrics.TotalTasks = AgentTasks.Num();
    Metrics.CompletedTasks = 0;
    Metrics.BlockedTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            Metrics.CompletedTasks++;
        }
        else if (Task.Status == EDir_AgentStatus::Blocked)
        {
            Metrics.BlockedTasks++;
        }
    }
    
    if (Metrics.TotalTasks > 0)
    {
        Metrics.OverallProgress = (float)Metrics.CompletedTasks / (float)Metrics.TotalTasks * 100.0f;
    }
    else
    {
        Metrics.OverallProgress = 0.0f;
    }
}

bool ADir_ProductionCoordinator::CheckDependencies(const FDir_AgentTask& Task)
{
    // Check if all dependencies are completed
    for (const FString& Dependency : Task.Dependencies)
    {
        bool DependencyMet = false;
        for (const FDir_AgentTask& OtherTask : AgentTasks)
        {
            if (OtherTask.AgentName == Dependency && OtherTask.Status == EDir_AgentStatus::Complete)
            {
                DependencyMet = true;
                break;
            }
        }
        
        if (!DependencyMet)
        {
            return false;
        }
    }
    
    return true;
}