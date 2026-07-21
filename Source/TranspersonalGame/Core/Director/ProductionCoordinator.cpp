#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogProductionCoordinator, Log, All);

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    bAutoUpdateMetrics = true;
    MetricsUpdateInterval = 5.0f;
    
    // Initialize production metrics
    ProductionMetrics = FDir_ProductionMetrics();
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogProductionCoordinator, Log, TEXT("Production Coordinator initialized for cycle: %s"), *ProductionMetrics.CurrentCycle);
    
    // Set up automatic metrics updates
    if (bAutoUpdateMetrics)
    {
        GetWorldTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &ADir_ProductionCoordinator::UpdateProductionMetrics,
            MetricsUpdateInterval,
            true
        );
    }
    
    // Initialize agent tasks for all 19 agents
    SetupAgentDependencies();
    
    UE_LOG(LogProductionCoordinator, Log, TEXT("Production Coordinator ready. Tracking %d agents."), ProductionMetrics.TotalAgents);
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Check dependencies and update agent readiness
    CheckDependencies();
}

void ADir_ProductionCoordinator::SetupAgentDependencies()
{
    // Clear existing tasks
    AgentTasks.Empty();
    AgentStatusMap.Empty();
    
    // Agent #1 - Studio Director (no dependencies)
    AddAgentTask(TEXT("Studio Director"), TEXT("Coordinate production pipeline and establish command structure"), 10.0f);
    
    // Agent #2 - Engine Architect (depends on #1)
    FDir_AgentTask EngineArchitectTask;
    EngineArchitectTask.AgentName = TEXT("Engine Architect");
    EngineArchitectTask.TaskDescription = TEXT("Define core architecture and technical framework");
    EngineArchitectTask.Priority = 9.0f;
    EngineArchitectTask.Dependencies.Add(TEXT("Studio Director"));
    AgentTasks.Add(EngineArchitectTask);
    
    // Agent #3 - Core Systems (depends on #2)
    FDir_AgentTask CoreSystemsTask;
    CoreSystemsTask.AgentName = TEXT("Core Systems Programmer");
    CoreSystemsTask.TaskDescription = TEXT("Implement physics, collision, and destruction systems");
    CoreSystemsTask.Priority = 8.0f;
    CoreSystemsTask.Dependencies.Add(TEXT("Engine Architect"));
    AgentTasks.Add(CoreSystemsTask);
    
    // Agent #4 - Performance Optimizer (depends on #3)
    FDir_AgentTask PerformanceTask;
    PerformanceTask.AgentName = TEXT("Performance Optimizer");
    PerformanceTask.TaskDescription = TEXT("Ensure 60fps PC / 30fps console performance");
    PerformanceTask.Priority = 8.0f;
    PerformanceTask.Dependencies.Add(TEXT("Core Systems Programmer"));
    AgentTasks.Add(PerformanceTask);
    
    // Agent #5 - World Generator (depends on #4)
    FDir_AgentTask WorldGenTask;
    WorldGenTask.AgentName = TEXT("Procedural World Generator");
    WorldGenTask.TaskDescription = TEXT("Generate terrain, biomes, and geographic structure");
    WorldGenTask.Priority = 7.0f;
    WorldGenTask.Dependencies.Add(TEXT("Performance Optimizer"));
    AgentTasks.Add(WorldGenTask);
    
    // Continue with remaining agents...
    TArray<FString> AgentNames = {
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
    
    // Set up remaining agents with sequential dependencies
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask Task;
        Task.AgentName = AgentNames[i];
        Task.TaskDescription = FString::Printf(TEXT("Execute specialized tasks for %s"), *AgentNames[i]);
        Task.Priority = 7.0f - (i * 0.1f); // Decreasing priority
        
        // Add dependency on previous agent
        if (i == 0)
        {
            Task.Dependencies.Add(TEXT("Procedural World Generator"));
        }
        else
        {
            Task.Dependencies.Add(AgentNames[i - 1]);
        }
        
        AgentTasks.Add(Task);
    }
    
    // Initialize status map
    UpdateAgentStatusMap();
    
    UE_LOG(LogProductionCoordinator, Log, TEXT("Initialized %d agent tasks with dependencies"), AgentTasks.Num());
}

void ADir_ProductionCoordinator::AddAgentTask(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Pending;
    NewTask.StartTime = FDateTime::Now();
    
    AgentTasks.Add(NewTask);
    AgentStatusMap.Add(AgentName, EDir_AgentStatus::Pending);
    
    UE_LOG(LogProductionCoordinator, Log, TEXT("Added task for agent: %s"), *AgentName);
}

void ADir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    // Find and update the agent task
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            
            if (NewStatus == EDir_AgentStatus::InProgress && Task.StartTime == FDateTime::MinValue())
            {
                Task.StartTime = FDateTime::Now();
            }
            else if (NewStatus == EDir_AgentStatus::Completed || NewStatus == EDir_AgentStatus::Failed)
            {
                Task.CompletionTime = FDateTime::Now();
            }
            
            break;
        }
    }
    
    // Update status map
    AgentStatusMap.Add(AgentName, NewStatus);
    
    UE_LOG(LogProductionCoordinator, Log, TEXT("Updated agent %s status to %d"), *AgentName, (int32)NewStatus);
    
    // Update metrics
    UpdateProductionMetrics();
}

void ADir_ProductionCoordinator::CompleteAgentTask(const FString& AgentName, const TArray<FString>& Deliverables)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = EDir_AgentStatus::Completed;
            Task.CompletionTime = FDateTime::Now();
            Task.Deliverables = Deliverables;
            
            UE_LOG(LogProductionCoordinator, Log, TEXT("Completed task for agent: %s with %d deliverables"), *AgentName, Deliverables.Num());
            break;
        }
    }
    
    UpdateAgentStatus(AgentName, EDir_AgentStatus::Completed);
}

void ADir_ProductionCoordinator::FailAgentTask(const FString& AgentName, const FString& FailureReason)
{
    UpdateAgentStatus(AgentName, EDir_AgentStatus::Failed);
    UE_LOG(LogProductionCoordinator, Warning, TEXT("Agent %s task failed: %s"), *AgentName, *FailureReason);
}

void ADir_ProductionCoordinator::UpdateProductionMetrics()
{
    ProductionMetrics.CompletedTasks = 0;
    ProductionMetrics.FailedTasks = 0;
    ProductionMetrics.ActiveAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Completed:
                ProductionMetrics.CompletedTasks++;
                break;
            case EDir_AgentStatus::Failed:
                ProductionMetrics.FailedTasks++;
                break;
            case EDir_AgentStatus::InProgress:
                ProductionMetrics.ActiveAgents++;
                break;
        }
    }
    
    // Calculate overall progress
    int32 TotalTasks = AgentTasks.Num();
    if (TotalTasks > 0)
    {
        ProductionMetrics.OverallProgress = (float)ProductionMetrics.CompletedTasks / (float)TotalTasks * 100.0f;
    }
    
    UpdateAgentStatusMap();
}

FDir_ProductionMetrics ADir_ProductionCoordinator::GetProductionMetrics() const
{
    return ProductionMetrics;
}

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetAgentTasks() const
{
    return AgentTasks;
}

TArray<FString> ADir_ProductionCoordinator::GetBlockedAgents() const
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

float ADir_ProductionCoordinator::GetCycleProgress() const
{
    return ProductionMetrics.OverallProgress;
}

TArray<FString> ADir_ProductionCoordinator::GetCriticalPath() const
{
    TArray<FString> CriticalPath;
    
    // Find the longest dependency chain
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status != EDir_AgentStatus::Completed)
        {
            CriticalPath.Add(Task.AgentName);
        }
    }
    
    return CriticalPath;
}

bool ADir_ProductionCoordinator::IsAgentReady(const FString& AgentName) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            return AreTaskDependenciesMet(Task);
        }
    }
    
    return false;
}

void ADir_ProductionCoordinator::InitializeProductionCycle(const FString& CycleID)
{
    ProductionMetrics.CurrentCycle = CycleID;
    ProductionMetrics.CycleStartTime = FDateTime::Now();
    
    // Reset all agent statuses to pending
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Pending;
        Task.StartTime = FDateTime::MinValue();
        Task.CompletionTime = FDateTime::MinValue();
        Task.Deliverables.Empty();
    }
    
    UpdateAgentStatusMap();
    UpdateProductionMetrics();
    
    UE_LOG(LogProductionCoordinator, Log, TEXT("Initialized new production cycle: %s"), *CycleID);
}

void ADir_ProductionCoordinator::LogProductionStatus() const
{
    UE_LOG(LogProductionCoordinator, Log, TEXT("=== PRODUCTION STATUS REPORT ==="));
    UE_LOG(LogProductionCoordinator, Log, TEXT("Cycle: %s"), *ProductionMetrics.CurrentCycle);
    UE_LOG(LogProductionCoordinator, Log, TEXT("Overall Progress: %.1f%%"), ProductionMetrics.OverallProgress);
    UE_LOG(LogProductionCoordinator, Log, TEXT("Completed Tasks: %d"), ProductionMetrics.CompletedTasks);
    UE_LOG(LogProductionCoordinator, Log, TEXT("Failed Tasks: %d"), ProductionMetrics.FailedTasks);
    UE_LOG(LogProductionCoordinator, Log, TEXT("Active Agents: %d"), ProductionMetrics.ActiveAgents);
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusString;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Pending: StatusString = TEXT("PENDING"); break;
            case EDir_AgentStatus::InProgress: StatusString = TEXT("IN PROGRESS"); break;
            case EDir_AgentStatus::Completed: StatusString = TEXT("COMPLETED"); break;
            case EDir_AgentStatus::Failed: StatusString = TEXT("FAILED"); break;
            case EDir_AgentStatus::Blocked: StatusString = TEXT("BLOCKED"); break;
        }
        
        UE_LOG(LogProductionCoordinator, Log, TEXT("Agent: %s - Status: %s - Priority: %.1f"), 
               *Task.AgentName, *StatusString, Task.Priority);
    }
    
    UE_LOG(LogProductionCoordinator, Log, TEXT("=== END REPORT ==="));
}

void ADir_ProductionCoordinator::ResetProductionCycle()
{
    InitializeProductionCycle(TEXT("RESET_CYCLE"));
    UE_LOG(LogProductionCoordinator, Log, TEXT("Production cycle reset"));
}

void ADir_ProductionCoordinator::CheckDependencies()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Pending)
        {
            if (AreTaskDependenciesMet(Task))
            {
                // Agent is ready to start
                Task.Status = EDir_AgentStatus::Pending; // Keep pending until actually started
            }
            else
            {
                // Agent is blocked by dependencies
                Task.Status = EDir_AgentStatus::Blocked;
            }
        }
    }
}

bool ADir_ProductionCoordinator::AreTaskDependenciesMet(const FDir_AgentTask& Task) const
{
    for (const FString& Dependency : Task.Dependencies)
    {
        bool DependencyMet = false;
        
        for (const FDir_AgentTask& OtherTask : AgentTasks)
        {
            if (OtherTask.AgentName == Dependency && OtherTask.Status == EDir_AgentStatus::Completed)
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

void ADir_ProductionCoordinator::UpdateAgentStatusMap()
{
    AgentStatusMap.Empty();
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        AgentStatusMap.Add(Task.AgentName, Task.Status);
    }
}