#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

UProductionCoordinator::UProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Initialize production metrics
    CurrentCycle = 28;
    TotalAgents = 19;
    ActiveAgents = 0;
    CompletedTasks = 0;
    PendingTasks = 0;
    CriticalIssues = 0;
    
    // Initialize agent status tracking
    AgentStatus.Empty();
    TaskQueue.Empty();
    ProductionMetrics.Empty();
    
    bIsProductionActive = false;
    bDebugMode = true;
}

void UProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize production coordination
    InitializeProductionCycle();
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Cycle %d initialized with %d agents"), 
               CurrentCycle, TotalAgents);
    }
}

void UProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, 
                                          FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsProductionActive)
    {
        UpdateProductionMetrics();
        CheckAgentStatus();
        ProcessTaskQueue();
    }
}

void UProductionCoordinator::InitializeProductionCycle()
{
    // Clear previous cycle data
    AgentStatus.Empty();
    TaskQueue.Empty();
    
    // Initialize agent tracking
    TArray<FString> AgentNames = {
        TEXT("Studio_Director"),
        TEXT("Engine_Architect"), 
        TEXT("Core_Systems"),
        TEXT("Performance_Optimizer"),
        TEXT("World_Generator"),
        TEXT("Environment_Artist"),
        TEXT("Architecture_Interior"),
        TEXT("Lighting_Atmosphere"),
        TEXT("Character_Artist"),
        TEXT("Animation_Agent"),
        TEXT("NPC_Behavior"),
        TEXT("Combat_Enemy_AI"),
        TEXT("Crowd_Traffic"),
        TEXT("Quest_Mission"),
        TEXT("Narrative_Dialogue"),
        TEXT("Audio_Agent"),
        TEXT("VFX_Agent"),
        TEXT("QA_Testing"),
        TEXT("Integration_Build")
    };
    
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentStatus NewStatus;
        NewStatus.AgentName = AgentName;
        NewStatus.Status = EDir_AgentState::Idle;
        NewStatus.CurrentTask = TEXT("Awaiting Assignment");
        NewStatus.Progress = 0.0f;
        NewStatus.LastUpdate = FDateTime::Now();
        NewStatus.Priority = EDir_TaskPriority::Medium;
        
        AgentStatus.Add(AgentName, NewStatus);
    }
    
    // Set critical agents for this cycle
    SetAgentPriority(TEXT("Core_Systems"), EDir_TaskPriority::Critical);
    SetAgentPriority(TEXT("World_Generator"), EDir_TaskPriority::High);
    SetAgentPriority(TEXT("Character_Artist"), EDir_TaskPriority::Critical);
    SetAgentPriority(TEXT("Combat_Enemy_AI"), EDir_TaskPriority::High);
    
    bIsProductionActive = true;
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Production cycle initialized"));
    }
}

void UProductionCoordinator::AssignTask(const FString& AgentName, const FString& TaskDescription, 
                                       EDir_TaskPriority Priority, float EstimatedDuration)
{
    if (!AgentStatus.Contains(AgentName))
    {
        UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Unknown agent %s"), *AgentName);
        return;
    }
    
    // Create new task
    FDir_ProductionTask NewTask;
    NewTask.TaskID = FGuid::NewGuid().ToString();
    NewTask.AgentName = AgentName;
    NewTask.Description = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_TaskStatus::Pending;
    NewTask.EstimatedDuration = EstimatedDuration;
    NewTask.ActualDuration = 0.0f;
    NewTask.CreatedTime = FDateTime::Now();
    NewTask.Dependencies.Empty();
    
    // Add to task queue
    TaskQueue.Add(NewTask);
    
    // Update agent status
    FDir_AgentStatus* Agent = AgentStatus.Find(AgentName);
    if (Agent)
    {
        Agent->Status = EDir_AgentState::Assigned;
        Agent->CurrentTask = TaskDescription;
        Agent->Priority = Priority;
        Agent->LastUpdate = FDateTime::Now();
    }
    
    PendingTasks++;
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Task assigned to %s: %s"), 
               *AgentName, *TaskDescription);
    }
}

void UProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentState NewStatus, 
                                              float Progress, const FString& StatusMessage)
{
    FDir_AgentStatus* Agent = AgentStatus.Find(AgentName);
    if (!Agent)
    {
        UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Unknown agent %s"), *AgentName);
        return;
    }
    
    EDir_AgentState PreviousStatus = Agent->Status;
    Agent->Status = NewStatus;
    Agent->Progress = FMath::Clamp(Progress, 0.0f, 100.0f);
    Agent->StatusMessage = StatusMessage;
    Agent->LastUpdate = FDateTime::Now();
    
    // Update task status if agent completed work
    if (NewStatus == EDir_AgentState::Completed && PreviousStatus != EDir_AgentState::Completed)
    {
        CompleteAgentTask(AgentName);
    }
    else if (NewStatus == EDir_AgentState::Error)
    {
        CriticalIssues++;
        UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Agent %s reported error: %s"), 
               *AgentName, *StatusMessage);
    }
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: %s status updated to %d (%.1f%%)"), 
               *AgentName, (int32)NewStatus, Progress);
    }
}

void UProductionCoordinator::CompleteAgentTask(const FString& AgentName)
{
    // Find and complete the agent's current task
    for (FDir_ProductionTask& Task : TaskQueue)
    {
        if (Task.AgentName == AgentName && Task.Status == EDir_TaskStatus::InProgress)
        {
            Task.Status = EDir_TaskStatus::Completed;
            Task.CompletedTime = FDateTime::Now();
            Task.ActualDuration = (Task.CompletedTime - Task.CreatedTime).GetTotalSeconds();
            
            CompletedTasks++;
            PendingTasks = FMath::Max(0, PendingTasks - 1);
            
            if (bDebugMode)
            {
                UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Task completed by %s in %.1f seconds"), 
                       *AgentName, Task.ActualDuration);
            }
            break;
        }
    }
}

FDir_ProductionMetrics UProductionCoordinator::GetProductionMetrics() const
{
    FDir_ProductionMetrics Metrics;
    Metrics.CurrentCycle = CurrentCycle;
    Metrics.TotalAgents = TotalAgents;
    Metrics.ActiveAgents = ActiveAgents;
    Metrics.CompletedTasks = CompletedTasks;
    Metrics.PendingTasks = PendingTasks;
    Metrics.CriticalIssues = CriticalIssues;
    
    // Calculate efficiency
    int32 TotalTasks = CompletedTasks + PendingTasks;
    Metrics.EfficiencyRating = TotalTasks > 0 ? (float)CompletedTasks / TotalTasks * 100.0f : 0.0f;
    
    return Metrics;
}

TArray<FDir_AgentStatus> UProductionCoordinator::GetAgentStatusList() const
{
    TArray<FDir_AgentStatus> StatusList;
    AgentStatus.GenerateValueArray(StatusList);
    return StatusList;
}

TArray<FDir_ProductionTask> UProductionCoordinator::GetTaskQueue() const
{
    return TaskQueue;
}

void UProductionCoordinator::SetAgentPriority(const FString& AgentName, EDir_TaskPriority Priority)
{
    FDir_AgentStatus* Agent = AgentStatus.Find(AgentName);
    if (Agent)
    {
        Agent->Priority = Priority;
        
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: %s priority set to %d"), 
                   *AgentName, (int32)Priority);
        }
    }
}

void UProductionCoordinator::UpdateProductionMetrics()
{
    ActiveAgents = 0;
    
    // Count active agents
    for (const auto& StatusPair : AgentStatus)
    {
        if (StatusPair.Value.Status == EDir_AgentState::Working || 
            StatusPair.Value.Status == EDir_AgentState::Assigned)
        {
            ActiveAgents++;
        }
    }
    
    // Update task statuses
    for (FDir_ProductionTask& Task : TaskQueue)
    {
        if (Task.Status == EDir_TaskStatus::Pending)
        {
            // Check if agent is ready to start
            FDir_AgentStatus* Agent = AgentStatus.Find(Task.AgentName);
            if (Agent && Agent->Status == EDir_AgentState::Assigned)
            {
                Task.Status = EDir_TaskStatus::InProgress;
                Agent->Status = EDir_AgentState::Working;
            }
        }
    }
}

void UProductionCoordinator::CheckAgentStatus()
{
    FDateTime CurrentTime = FDateTime::Now();
    
    for (auto& StatusPair : AgentStatus)
    {
        FDir_AgentStatus& Agent = StatusPair.Value;
        
        // Check for stalled agents (no update in 5 minutes)
        double TimeSinceUpdate = (CurrentTime - Agent.LastUpdate).GetTotalMinutes();
        if (TimeSinceUpdate > 5.0 && Agent.Status == EDir_AgentState::Working)
        {
            Agent.Status = EDir_AgentState::Stalled;
            
            if (bDebugMode)
            {
                UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent %s appears stalled"), 
                       *Agent.AgentName);
            }
        }
    }
}

void UProductionCoordinator::ProcessTaskQueue()
{
    // Sort tasks by priority and process high-priority tasks first
    TaskQueue.Sort([](const FDir_ProductionTask& A, const FDir_ProductionTask& B) {
        return (int32)A.Priority > (int32)B.Priority;
    });
    
    // Process pending tasks
    for (FDir_ProductionTask& Task : TaskQueue)
    {
        if (Task.Status == EDir_TaskStatus::Pending)
        {
            // Check dependencies
            bool CanStart = true;
            for (const FString& Dependency : Task.Dependencies)
            {
                // Find dependency task
                bool DependencyCompleted = false;
                for (const FDir_ProductionTask& DepTask : TaskQueue)
                {
                    if (DepTask.TaskID == Dependency && DepTask.Status == EDir_TaskStatus::Completed)
                    {
                        DependencyCompleted = true;
                        break;
                    }
                }
                
                if (!DependencyCompleted)
                {
                    CanStart = false;
                    break;
                }
            }
            
            if (CanStart)
            {
                // Start the task
                FDir_AgentStatus* Agent = AgentStatus.Find(Task.AgentName);
                if (Agent && Agent->Status == EDir_AgentState::Assigned)
                {
                    Task.Status = EDir_TaskStatus::InProgress;
                    Agent->Status = EDir_AgentState::Working;
                    Agent->Progress = 0.0f;
                    
                    if (bDebugMode)
                    {
                        UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Started task for %s"), 
                               *Task.AgentName);
                    }
                }
            }
        }
    }
}