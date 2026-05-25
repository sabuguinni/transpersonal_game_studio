#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"

UProductionCoordinator::UProductionCoordinator()
{
    CurrentMetrics = FDir_ProductionMetrics();
}

void UProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing 19-agent development pipeline"));
    
    // Initialize agent registry
    RegisterAgent(TEXT("Studio Director"), 1);
    RegisterAgent(TEXT("Engine Architect"), 2);
    RegisterAgent(TEXT("Core Systems Programmer"), 3);
    RegisterAgent(TEXT("Performance Optimizer"), 4);
    RegisterAgent(TEXT("Procedural World Generator"), 5);
    RegisterAgent(TEXT("Environment Artist"), 6);
    RegisterAgent(TEXT("Architecture & Interior Agent"), 7);
    RegisterAgent(TEXT("Lighting & Atmosphere Agent"), 8);
    RegisterAgent(TEXT("Character Artist Agent"), 9);
    RegisterAgent(TEXT("Animation Agent"), 10);
    RegisterAgent(TEXT("NPC Behavior Agent"), 11);
    RegisterAgent(TEXT("Combat & Enemy AI Agent"), 12);
    RegisterAgent(TEXT("Crowd & Traffic Simulation"), 13);
    RegisterAgent(TEXT("Quest & Mission Designer"), 14);
    RegisterAgent(TEXT("Narrative & Dialogue Agent"), 15);
    RegisterAgent(TEXT("Audio Agent"), 16);
    RegisterAgent(TEXT("VFX Agent"), 17);
    RegisterAgent(TEXT("QA & Testing Agent"), 18);
    RegisterAgent(TEXT("Integration & Build Agent"), 19);
    
    SetCurrentMilestone(TEXT("Playable Prototype - Walk Around"));
    UpdateMetrics();
}

void UProductionCoordinator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Shutting down"));
    Super::Deinitialize();
}

void UProductionCoordinator::RegisterAgent(const FString& AgentName, int32 AgentNumber)
{
    AgentNumbers.Add(AgentName, AgentNumber);
    AgentStatuses.Add(AgentName, EDir_AgentStatus::Idle);
    AgentTasks.Add(AgentName, TArray<FDir_AgentTask>());
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Registered Agent #%d - %s"), AgentNumber, *AgentName);
}

void UProductionCoordinator::AssignTask(const FString& AgentName, const FDir_AgentTask& Task)
{
    if (!AgentTasks.Contains(AgentName))
    {
        UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Agent %s not found"), *AgentName);
        return;
    }
    
    // Check if agent is blocked
    if (AgentStatuses[AgentName] == EDir_AgentStatus::Blocked)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent %s is blocked - %s"), 
               *AgentName, *BlockedReasons.FindRef(AgentName));
        return;
    }
    
    // Check dependencies
    if (!CheckDependencies(Task))
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Task dependencies not met for %s"), *AgentName);
        return;
    }
    
    AgentTasks[AgentName].Add(Task);
    AgentStatuses[AgentName] = EDir_AgentStatus::Working;
    
    LogTaskUpdate(AgentName, Task.TaskDescription, EDir_AgentStatus::Working);
    UpdateMetrics();
}

void UProductionCoordinator::UpdateTaskStatus(const FString& AgentName, const FString& TaskDescription, EDir_AgentStatus NewStatus)
{
    if (!AgentTasks.Contains(AgentName))
    {
        return;
    }
    
    TArray<FDir_AgentTask>& Tasks = AgentTasks[AgentName];
    for (FDir_AgentTask& Task : Tasks)
    {
        if (Task.TaskDescription == TaskDescription)
        {
            Task.Status = NewStatus;
            LogTaskUpdate(AgentName, TaskDescription, NewStatus);
            break;
        }
    }
    
    // Update agent status based on tasks
    bool bHasWorkingTasks = false;
    bool bAllCompleted = true;
    
    for (const FDir_AgentTask& Task : Tasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            bHasWorkingTasks = true;
            bAllCompleted = false;
        }
        else if (Task.Status != EDir_AgentStatus::Completed)
        {
            bAllCompleted = false;
        }
    }
    
    if (bHasWorkingTasks)
    {
        AgentStatuses[AgentName] = EDir_AgentStatus::Working;
    }
    else if (bAllCompleted && Tasks.Num() > 0)
    {
        AgentStatuses[AgentName] = EDir_AgentStatus::Completed;
    }
    else
    {
        AgentStatuses[AgentName] = EDir_AgentStatus::Idle;
    }
    
    UpdateMetrics();
}

bool UProductionCoordinator::CanAgentStart(const FString& AgentName) const
{
    if (!AgentStatuses.Contains(AgentName))
    {
        return false;
    }
    
    EDir_AgentStatus Status = AgentStatuses[AgentName];
    return Status == EDir_AgentStatus::Idle || Status == EDir_AgentStatus::Completed;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const auto& AgentTaskPair : AgentTasks)
    {
        for (const FDir_AgentTask& Task : AgentTaskPair.Value)
        {
            if (Task.Status == EDir_AgentStatus::Idle || Task.Status == EDir_AgentStatus::Working)
            {
                PendingTasks.Add(Task);
            }
        }
    }
    
    return PendingTasks;
}

FDir_ProductionMetrics UProductionCoordinator::GetProductionMetrics() const
{
    return CurrentMetrics;
}

void UProductionCoordinator::SetHighPriorityTask(const FString& AgentName, const FString& TaskDescription)
{
    if (!AgentTasks.Contains(AgentName))
    {
        return;
    }
    
    TArray<FDir_AgentTask>& Tasks = AgentTasks[AgentName];
    for (FDir_AgentTask& Task : Tasks)
    {
        if (Task.TaskDescription == TaskDescription)
        {
            Task.Priority = 10.0f; // Max priority
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Set HIGH PRIORITY - %s: %s"), 
                   *AgentName, *TaskDescription);
            break;
        }
    }
}

void UProductionCoordinator::BlockAgent(const FString& AgentName, const FString& Reason)
{
    if (AgentStatuses.Contains(AgentName))
    {
        AgentStatuses[AgentName] = EDir_AgentStatus::Blocked;
        BlockedReasons.Add(AgentName, Reason);
        UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: BLOCKED %s - %s"), *AgentName, *Reason);
        UpdateMetrics();
    }
}

void UProductionCoordinator::UnblockAgent(const FString& AgentName)
{
    if (AgentStatuses.Contains(AgentName))
    {
        AgentStatuses[AgentName] = EDir_AgentStatus::Idle;
        BlockedReasons.Remove(AgentName);
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: UNBLOCKED %s"), *AgentName);
        UpdateMetrics();
    }
}

void UProductionCoordinator::SetCurrentMilestone(const FString& MilestoneName)
{
    CurrentMetrics.CurrentMilestone = MilestoneName;
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Current Milestone - %s"), *MilestoneName);
}

float UProductionCoordinator::CalculateOverallProgress() const
{
    int32 TotalTasks = 0;
    int32 CompletedTasks = 0;
    
    for (const auto& AgentTaskPair : AgentTasks)
    {
        for (const FDir_AgentTask& Task : AgentTaskPair.Value)
        {
            TotalTasks++;
            if (Task.Status == EDir_AgentStatus::Completed)
            {
                CompletedTasks++;
            }
        }
    }
    
    return TotalTasks > 0 ? (float)CompletedTasks / (float)TotalTasks * 100.0f : 0.0f;
}

void UProductionCoordinator::PrintProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Milestone: %s"), *CurrentMetrics.CurrentMilestone);
    UE_LOG(LogTemp, Warning, TEXT("Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d/%d"), CurrentMetrics.ActiveAgents, CurrentMetrics.TotalAgents);
    
    for (const auto& AgentStatusPair : AgentStatuses)
    {
        FString StatusText;
        switch (AgentStatusPair.Value)
        {
            case EDir_AgentStatus::Idle: StatusText = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusText = TEXT("WORKING"); break;
            case EDir_AgentStatus::Completed: StatusText = TEXT("COMPLETED"); break;
            case EDir_AgentStatus::Failed: StatusText = TEXT("FAILED"); break;
            case EDir_AgentStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
        }
        
        int32 AgentNumber = AgentNumbers.FindRef(AgentStatusPair.Key);
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d %s: %s"), AgentNumber, *AgentStatusPair.Key, *StatusText);
    }
}

void UProductionCoordinator::ResetAllTasks()
{
    for (auto& AgentTaskPair : AgentTasks)
    {
        AgentTaskPair.Value.Empty();
    }
    
    for (auto& AgentStatusPair : AgentStatuses)
    {
        AgentStatusPair.Value = EDir_AgentStatus::Idle;
    }
    
    BlockedReasons.Empty();
    UpdateMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: All tasks reset"));
}

void UProductionCoordinator::UpdateMetrics()
{
    CurrentMetrics.TotalAgents = AgentStatuses.Num();
    CurrentMetrics.ActiveAgents = 0;
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.FailedTasks = 0;
    
    for (const auto& AgentStatusPair : AgentStatuses)
    {
        if (AgentStatusPair.Value == EDir_AgentStatus::Working)
        {
            CurrentMetrics.ActiveAgents++;
        }
    }
    
    for (const auto& AgentTaskPair : AgentTasks)
    {
        for (const FDir_AgentTask& Task : AgentTaskPair.Value)
        {
            if (Task.Status == EDir_AgentStatus::Completed)
            {
                CurrentMetrics.CompletedTasks++;
            }
            else if (Task.Status == EDir_AgentStatus::Failed)
            {
                CurrentMetrics.FailedTasks++;
            }
        }
    }
    
    CurrentMetrics.OverallProgress = CalculateOverallProgress();
}

bool UProductionCoordinator::CheckDependencies(const FDir_AgentTask& Task) const
{
    for (const FString& Dependency : Task.Dependencies)
    {
        bool bDependencyMet = false;
        
        for (const auto& AgentTaskPair : AgentTasks)
        {
            for (const FDir_AgentTask& ExistingTask : AgentTaskPair.Value)
            {
                if (ExistingTask.TaskDescription.Contains(Dependency) && 
                    ExistingTask.Status == EDir_AgentStatus::Completed)
                {
                    bDependencyMet = true;
                    break;
                }
            }
            if (bDependencyMet) break;
        }
        
        if (!bDependencyMet)
        {
            return false;
        }
    }
    
    return true;
}

void UProductionCoordinator::LogTaskUpdate(const FString& AgentName, const FString& TaskDescription, EDir_AgentStatus Status)
{
    FString StatusText;
    switch (Status)
    {
        case EDir_AgentStatus::Idle: StatusText = TEXT("IDLE"); break;
        case EDir_AgentStatus::Working: StatusText = TEXT("WORKING"); break;
        case EDir_AgentStatus::Completed: StatusText = TEXT("COMPLETED"); break;
        case EDir_AgentStatus::Failed: StatusText = TEXT("FAILED"); break;
        case EDir_AgentStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: %s - %s [%s]"), 
           *AgentName, *TaskDescription, *StatusText);
}