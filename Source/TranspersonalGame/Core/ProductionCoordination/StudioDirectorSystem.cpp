#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogStudioDirector, Log, All);

UStudioDirectorSystem::UStudioDirectorSystem()
{
    ProductionMetrics = FDir_ProductionMetrics();
    CurrentCycleID = TEXT("");
    CycleStartTime = FDateTime::Now();
}

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeAgentSlots();
    
    UE_LOG(LogStudioDirector, Log, TEXT("Studio Director System initialized - Ready to coordinate 19 agents"));
    LogProductionEvent(TEXT("Studio Director System Online"));
}

void UStudioDirectorSystem::Deinitialize()
{
    EmergencyStopAllAgents();
    
    UE_LOG(LogStudioDirector, Log, TEXT("Studio Director System shutdown"));
    LogProductionEvent(TEXT("Studio Director System Offline"));
    
    Super::Deinitialize();
}

void UStudioDirectorSystem::StartProductionCycle(const FString& CycleID)
{
    if (CycleID.IsEmpty())
    {
        UE_LOG(LogStudioDirector, Warning, TEXT("Cannot start production cycle with empty ID"));
        return;
    }

    CurrentCycleID = CycleID;
    CycleStartTime = FDateTime::Now();
    ProductionMetrics.TotalCycles++;
    
    // Reset all agent statuses to Idle for new cycle
    for (auto& AgentPair : AgentTasks)
    {
        AgentPair.Value.Status = EDir_AgentStatus::Idle;
        AgentPair.Value.StartTime = FDateTime::Now();
    }
    
    UpdateProductionMetrics();
    
    UE_LOG(LogStudioDirector, Log, TEXT("Production Cycle Started: %s"), *CycleID);
    LogProductionEvent(FString::Printf(TEXT("Cycle Started: %s"), *CycleID));
    
    OnProductionCycleStarted(CycleID);
}

void UStudioDirectorSystem::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority)
{
    if (!ValidateAgentNumber(AgentNumber))
    {
        UE_LOG(LogStudioDirector, Error, TEXT("Invalid agent number: %d"), AgentNumber);
        return;
    }

    if (TaskDescription.IsEmpty())
    {
        UE_LOG(LogStudioDirector, Warning, TEXT("Cannot assign empty task to agent %d"), AgentNumber);
        return;
    }

    FDir_AgentTask& Task = AgentTasks[AgentNumber];
    Task.TaskDescription = TaskDescription;
    Task.Priority = Priority;
    Task.Status = EDir_AgentStatus::Working;
    Task.StartTime = FDateTime::Now();
    Task.DeadlineTime = FDateTime::Now() + FTimespan::FromMinutes(30);
    
    UpdateProductionMetrics();
    
    UE_LOG(LogStudioDirector, Log, TEXT("Task assigned to Agent #%d: %s (Priority: %.2f)"), 
           AgentNumber, *TaskDescription, Priority);
    LogProductionEvent(FString::Printf(TEXT("Agent #%d assigned: %s"), AgentNumber, *TaskDescription));
}

void UStudioDirectorSystem::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus)
{
    if (!ValidateAgentNumber(AgentNumber))
    {
        return;
    }

    FDir_AgentTask& Task = AgentTasks[AgentNumber];
    EDir_AgentStatus OldStatus = Task.Status;
    Task.Status = NewStatus;
    
    // Update metrics based on status change
    if (OldStatus == EDir_AgentStatus::Working && NewStatus == EDir_AgentStatus::Completed)
    {
        ProductionMetrics.CompletedTasks++;
        
        // Calculate task completion time
        FTimespan TaskDuration = FDateTime::Now() - Task.StartTime;
        float TaskTimeMinutes = TaskDuration.GetTotalMinutes();
        
        // Update average task time
        if (ProductionMetrics.CompletedTasks > 0)
        {
            ProductionMetrics.AverageTaskTime = 
                (ProductionMetrics.AverageTaskTime * (ProductionMetrics.CompletedTasks - 1) + TaskTimeMinutes) 
                / ProductionMetrics.CompletedTasks;
        }
    }
    else if (NewStatus == EDir_AgentStatus::Failed || NewStatus == EDir_AgentStatus::Timeout)
    {
        ProductionMetrics.FailedTasks++;
    }
    
    UpdateProductionMetrics();
    
    UE_LOG(LogStudioDirector, Log, TEXT("Agent #%d status changed: %s -> %s"), 
           AgentNumber, 
           *UEnum::GetValueAsString(OldStatus),
           *UEnum::GetValueAsString(NewStatus));
}

void UStudioDirectorSystem::CompleteAgentTask(int32 AgentNumber, const TArray<FString>& Deliverables)
{
    if (!ValidateAgentNumber(AgentNumber))
    {
        return;
    }

    FDir_AgentTask& Task = AgentTasks[AgentNumber];
    Task.Status = EDir_AgentStatus::Completed;
    Task.Deliverables = Deliverables;
    
    UpdateAgentStatus(AgentNumber, EDir_AgentStatus::Completed);
    
    FString DeliverablesStr = FString::Join(Deliverables, TEXT(", "));
    UE_LOG(LogStudioDirector, Log, TEXT("Agent #%d completed task: %s | Deliverables: %s"), 
           AgentNumber, *Task.TaskDescription, *DeliverablesStr);
    
    OnAgentTaskCompleted(AgentNumber, Task.TaskDescription);
}

FDir_ProductionMetrics UStudioDirectorSystem::GetProductionMetrics() const
{
    return ProductionMetrics;
}

TArray<FDir_AgentTask> UStudioDirectorSystem::GetActiveAgentTasks() const
{
    TArray<FDir_AgentTask> ActiveTasks;
    
    for (const auto& AgentPair : AgentTasks)
    {
        if (AgentPair.Value.Status == EDir_AgentStatus::Working)
        {
            ActiveTasks.Add(AgentPair.Value);
        }
    }
    
    return ActiveTasks;
}

bool UStudioDirectorSystem::IsAgentAvailable(int32 AgentNumber) const
{
    if (!ValidateAgentNumber(AgentNumber))
    {
        return false;
    }

    const FDir_AgentTask* Task = AgentTasks.Find(AgentNumber);
    if (!Task)
    {
        return false;
    }

    return Task->Status == EDir_AgentStatus::Idle || Task->Status == EDir_AgentStatus::Completed;
}

void UStudioDirectorSystem::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    EDir_ProductionPhase OldPhase = ProductionMetrics.CurrentPhase;
    ProductionMetrics.CurrentPhase = NewPhase;
    
    UpdateProductionMetrics();
    
    UE_LOG(LogStudioDirector, Log, TEXT("Production phase changed: %s -> %s"), 
           *UEnum::GetValueAsString(OldPhase),
           *UEnum::GetValueAsString(NewPhase));
    
    OnProductionPhaseChanged(NewPhase);
}

void UStudioDirectorSystem::EmergencyStopAllAgents()
{
    int32 StoppedAgents = 0;
    
    for (auto& AgentPair : AgentTasks)
    {
        if (AgentPair.Value.Status == EDir_AgentStatus::Working)
        {
            AgentPair.Value.Status = EDir_AgentStatus::Failed;
            StoppedAgents++;
        }
    }
    
    UE_LOG(LogStudioDirector, Warning, TEXT("EMERGENCY STOP: %d agents stopped"), StoppedAgents);
    LogProductionEvent(FString::Printf(TEXT("EMERGENCY STOP: %d agents"), StoppedAgents));
}

void UStudioDirectorSystem::RestartFailedTasks()
{
    int32 RestartedTasks = 0;
    
    for (auto& AgentPair : AgentTasks)
    {
        if (AgentPair.Value.Status == EDir_AgentStatus::Failed || 
            AgentPair.Value.Status == EDir_AgentStatus::Timeout)
        {
            AgentPair.Value.Status = EDir_AgentStatus::Idle;
            AgentPair.Value.StartTime = FDateTime::Now();
            RestartedTasks++;
        }
    }
    
    UE_LOG(LogStudioDirector, Log, TEXT("Restarted %d failed tasks"), RestartedTasks);
    LogProductionEvent(FString::Printf(TEXT("Restarted %d failed tasks"), RestartedTasks));
}

void UStudioDirectorSystem::InitializeAgentSlots()
{
    // Initialize all 19 agent slots
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

    for (int32 i = 1; i <= 19; i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentNumber = i;
        NewTask.AgentName = (i <= AgentNames.Num()) ? AgentNames[i-1] : FString::Printf(TEXT("Agent #%d"), i);
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = 1.0f;
        
        AgentTasks.Add(i, NewTask);
    }
    
    UE_LOG(LogStudioDirector, Log, TEXT("Initialized %d agent slots"), AgentTasks.Num());
}

void UStudioDirectorSystem::UpdateProductionMetrics()
{
    // Count active agents
    ProductionMetrics.ActiveAgents = 0;
    for (const auto& AgentPair : AgentTasks)
    {
        if (AgentPair.Value.Status == EDir_AgentStatus::Working)
        {
            ProductionMetrics.ActiveAgents++;
        }
    }
    
    // Calculate overall progress based on completed vs total tasks
    int32 TotalTasks = ProductionMetrics.CompletedTasks + ProductionMetrics.FailedTasks + ProductionMetrics.ActiveAgents;
    if (TotalTasks > 0)
    {
        ProductionMetrics.OverallProgress = (float)ProductionMetrics.CompletedTasks / TotalTasks * 100.0f;
    }
}

bool UStudioDirectorSystem::ValidateAgentNumber(int32 AgentNumber) const
{
    return AgentNumber >= 1 && AgentNumber <= 19;
}

void UStudioDirectorSystem::LogProductionEvent(const FString& Event) const
{
    FString LogMessage = FString::Printf(TEXT("[%s] %s"), *CurrentCycleID, *Event);
    UE_LOG(LogStudioDirector, Log, TEXT("%s"), *LogMessage);
    
    // Also log to screen for immediate visibility
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, LogMessage);
    }
}