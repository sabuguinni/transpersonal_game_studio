#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UProductionCoordinator::UProductionCoordinator()
{
    CurrentMetrics = FDir_ProductionMetrics();
}

void UProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeAgentChain();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized - Studio Director ready"));
    
    // Set initial milestone
    SetCurrentMilestone(TEXT("Milestone 1 - Walk Around"));
    
    // Log startup
    LogProductionEvent(TEXT("Production Coordinator System Online - 19 Agent Chain Ready"));
}

void UProductionCoordinator::Deinitialize()
{
    ActiveTasks.Empty();
    ProductionLog.Empty();
    
    Super::Deinitialize();
}

void UProductionCoordinator::InitializeAgentChain()
{
    // Define the 19-agent production chain
    AgentChainOrder.Empty();
    AgentChainOrder.Add(TEXT("Studio Director"), 1);
    AgentChainOrder.Add(TEXT("Engine Architect"), 2);
    AgentChainOrder.Add(TEXT("Core Systems Programmer"), 3);
    AgentChainOrder.Add(TEXT("Performance Optimizer"), 4);
    AgentChainOrder.Add(TEXT("Procedural World Generator"), 5);
    AgentChainOrder.Add(TEXT("Environment Artist"), 6);
    AgentChainOrder.Add(TEXT("Architecture & Interior Agent"), 7);
    AgentChainOrder.Add(TEXT("Lighting & Atmosphere Agent"), 8);
    AgentChainOrder.Add(TEXT("Character Artist Agent"), 9);
    AgentChainOrder.Add(TEXT("Animation Agent"), 10);
    AgentChainOrder.Add(TEXT("NPC Behavior Agent"), 11);
    AgentChainOrder.Add(TEXT("Combat & Enemy AI Agent"), 12);
    AgentChainOrder.Add(TEXT("Crowd & Traffic Simulation"), 13);
    AgentChainOrder.Add(TEXT("Quest & Mission Designer"), 14);
    AgentChainOrder.Add(TEXT("Narrative & Dialogue Agent"), 15);
    AgentChainOrder.Add(TEXT("Audio Agent"), 16);
    AgentChainOrder.Add(TEXT("VFX Agent"), 17);
    AgentChainOrder.Add(TEXT("QA & Testing Agent"), 18);
    AgentChainOrder.Add(TEXT("Integration & Build Agent"), 19);
}

void UProductionCoordinator::AssignTask(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.StartTime = FDateTime::Now();
    NewTask.DeadlineTime = FDateTime::Now() + FTimespan::FromHours(2);
    
    ActiveTasks.Add(NewTask);
    UpdateMetrics();
    
    FString LogMessage = FString::Printf(TEXT("TASK ASSIGNED: %s -> %s (Priority: %.1f)"), 
        *AgentName, *TaskDescription, Priority);
    LogProductionEvent(LogMessage);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription);
}

void UProductionCoordinator::UpdateTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName && Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = NewStatus;
            
            FString StatusString;
            switch (NewStatus)
            {
                case EDir_AgentStatus::Completed: StatusString = TEXT("COMPLETED"); break;
                case EDir_AgentStatus::Failed: StatusString = TEXT("FAILED"); break;
                case EDir_AgentStatus::Blocked: StatusString = TEXT("BLOCKED"); break;
                default: StatusString = TEXT("UNKNOWN"); break;
            }
            
            FString LogMessage = FString::Printf(TEXT("TASK UPDATE: %s -> %s"), 
                *AgentName, *StatusString);
            LogProductionEvent(LogMessage);
            
            break;
        }
    }
    
    UpdateMetrics();
}

TArray<FDir_AgentTask> UProductionCoordinator::GetTasksForAgent(const FString& AgentName)
{
    TArray<FDir_AgentTask> AgentTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetAllTasks()
{
    return ActiveTasks;
}

FDir_ProductionMetrics UProductionCoordinator::GetProductionMetrics()
{
    UpdateMetrics();
    return CurrentMetrics;
}

void UProductionCoordinator::UpdateMetrics()
{
    CurrentMetrics.TotalTasks = ActiveTasks.Num();
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.FailedTasks = 0;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
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
    
    if (CurrentMetrics.TotalTasks > 0)
    {
        CurrentMetrics.OverallProgress = (float)CurrentMetrics.CompletedTasks / (float)CurrentMetrics.TotalTasks * 100.0f;
    }
    else
    {
        CurrentMetrics.OverallProgress = 0.0f;
    }
}

void UProductionCoordinator::LogProductionEvent(const FString& EventDescription)
{
    FString TimestampedEvent = FString::Printf(TEXT("[%s] %s"), 
        *FDateTime::Now().ToString(), *EventDescription);
    
    ProductionLog.Add(TimestampedEvent);
    
    // Keep only last 100 events to prevent memory bloat
    if (ProductionLog.Num() > 100)
    {
        ProductionLog.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("PRODUCTION LOG: %s"), *EventDescription);
}

void UProductionCoordinator::SetCurrentMilestone(const FString& MilestoneName)
{
    CurrentMetrics.CurrentMilestone = MilestoneName;
    
    FString LogMessage = FString::Printf(TEXT("MILESTONE SET: %s"), *MilestoneName);
    LogProductionEvent(LogMessage);
}

bool UProductionCoordinator::IsMilestoneComplete(const FString& MilestoneName)
{
    if (MilestoneName == TEXT("Milestone 1 - Walk Around"))
    {
        // Check if core playable prototype requirements are met
        int32 CoreSystemsComplete = 0;
        
        for (const FDir_AgentTask& Task : ActiveTasks)
        {
            if (Task.Status == EDir_AgentStatus::Completed)
            {
                if (Task.AgentName.Contains(TEXT("Character")) || 
                    Task.AgentName.Contains(TEXT("Movement")) ||
                    Task.AgentName.Contains(TEXT("Camera")))
                {
                    CoreSystemsComplete++;
                }
            }
        }
        
        return CoreSystemsComplete >= 3; // Minimum viable prototype
    }
    
    return false;
}

void UProductionCoordinator::TriggerNextAgent(const FString& CurrentAgent)
{
    FString NextAgent = GetNextAgentInChain(CurrentAgent);
    
    if (!NextAgent.IsEmpty())
    {
        FString LogMessage = FString::Printf(TEXT("CHAIN TRIGGER: %s -> %s"), 
            *CurrentAgent, *NextAgent);
        LogProductionEvent(LogMessage);
        
        // Auto-assign next agent task based on chain position
        FString NextTaskDescription = FString::Printf(TEXT("Continue production chain from %s"), *CurrentAgent);
        AssignTask(NextAgent, NextTaskDescription, 5.0f); // High priority for chain continuity
    }
}

FString UProductionCoordinator::GetNextAgentInChain(const FString& CurrentAgent)
{
    int32* CurrentOrder = AgentChainOrder.Find(CurrentAgent);
    if (!CurrentOrder)
    {
        return TEXT("");
    }
    
    int32 NextOrder = *CurrentOrder + 1;
    if (NextOrder > 19)
    {
        return TEXT("Studio Director"); // Loop back to start
    }
    
    for (const TPair<FString, int32>& AgentPair : AgentChainOrder)
    {
        if (AgentPair.Value == NextOrder)
        {
            return AgentPair.Key;
        }
    }
    
    return TEXT("");
}

void UProductionCoordinator::ValidateTaskDependencies()
{
    // Check for blocked tasks that can now proceed
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            // Simple dependency resolution - if previous agent in chain is complete
            FString PreviousAgent = GetNextAgentInChain(Task.AgentName); // This gets next, we need previous
            // Implementation would check if dependencies are resolved
            
            // For now, just log the check
            FString LogMessage = FString::Printf(TEXT("DEPENDENCY CHECK: %s"), *Task.AgentName);
            LogProductionEvent(LogMessage);
        }
    }
}