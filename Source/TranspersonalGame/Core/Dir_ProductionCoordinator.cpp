#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    bProductionActive = false;
    CurrentCycleID = TEXT("");
    CycleTimeoutMinutes = 30.0f; // 30 minute cycle timeout
    
    // Initialize default metrics
    CurrentMetrics = FDir_ProductionMetrics();
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized"));
    
    // Initialize the 18-agent pipeline
    TArray<FString> AgentNames = {
        TEXT("Agent02_EngineArchitect"),
        TEXT("Agent03_CoreSystems"),
        TEXT("Agent04_Performance"),
        TEXT("Agent05_WorldGeneration"),
        TEXT("Agent06_EnvironmentArt"),
        TEXT("Agent07_Architecture"),
        TEXT("Agent08_Lighting"),
        TEXT("Agent09_Characters"),
        TEXT("Agent10_Animation"),
        TEXT("Agent11_NPCBehavior"),
        TEXT("Agent12_CombatAI"),
        TEXT("Agent13_CrowdSimulation"),
        TEXT("Agent14_QuestDesign"),
        TEXT("Agent15_Narrative"),
        TEXT("Agent16_Audio"),
        TEXT("Agent17_VFX"),
        TEXT("Agent18_QA"),
        TEXT("Agent19_Integration")
    };
    
    // Create initial task structure for all agents
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentName;
        NewTask.TaskDescription = FString::Printf(TEXT("Awaiting task assignment for %s"), *AgentName);
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = 1.0f;
        AgentTasks.Add(NewTask);
    }
    
    CurrentMetrics.TotalAgents = AgentTasks.Num();
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bProductionActive)
    {
        UpdateProductionMetrics();
        CheckForTimeouts();
        ValidateAgentChain();
    }
}

void UDir_ProductionCoordinator::InitializeProductionCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    bProductionActive = true;
    CurrentMetrics.CycleStartTime = FDateTime::Now();
    CurrentMetrics.CurrentPhase = EDir_ProductionPhase::PreProduction;
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle %s initialized"), *CycleID);
    
    // Reset all agent statuses
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.StartTime = FDateTime::Now();
        Task.DeadlineTime = FDateTime::Now() + FTimespan::FromMinutes(CycleTimeoutMinutes);
    }
    
    OnCycleStarted(CycleID);
}

void UDir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        Task->TaskDescription = TaskDescription;
        Task->Priority = Priority;
        Task->Status = EDir_AgentStatus::Working;
        Task->StartTime = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find agent: %s"), *AgentName);
    }
}

void UDir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        EDir_AgentStatus OldStatus = Task->Status;
        Task->Status = NewStatus;
        
        UE_LOG(LogTemp, Log, TEXT("Agent %s status changed from %d to %d"), 
               *AgentName, (int32)OldStatus, (int32)NewStatus);
               
        if (NewStatus == EDir_AgentStatus::Completed)
        {
            OnAgentCompleted(AgentName);
        }
    }
}

void UDir_ProductionCoordinator::CompleteAgentTask(const FString& AgentName, const TArray<FString>& Deliverables)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        Task->Status = EDir_AgentStatus::Completed;
        Task->Deliverables = Deliverables;
        
        UE_LOG(LogTemp, Warning, TEXT("Agent %s completed task with %d deliverables"), 
               *AgentName, Deliverables.Num());
               
        OnAgentCompleted(AgentName);
    }
}

bool UDir_ProductionCoordinator::CheckAgentDependencies(const FString& AgentName)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (!Task)
    {
        return false;
    }
    
    // Check if all dependencies are completed
    for (const FString& Dependency : Task->Dependencies)
    {
        FDir_AgentTask* DepTask = FindAgentTask(Dependency);
        if (!DepTask || DepTask->Status != EDir_AgentStatus::Completed)
        {
            UE_LOG(LogTemp, Warning, TEXT("Agent %s blocked by dependency: %s"), 
                   *AgentName, *Dependency);
            return false;
        }
    }
    
    return true;
}

void UDir_ProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    EDir_ProductionPhase OldPhase = CurrentMetrics.CurrentPhase;
    CurrentMetrics.CurrentPhase = NewPhase;
    
    UE_LOG(LogTemp, Warning, TEXT("Production phase changed from %d to %d"), 
           (int32)OldPhase, (int32)NewPhase);
           
    OnProductionPhaseChanged(NewPhase);
}

FDir_ProductionMetrics UDir_ProductionCoordinator::GetCurrentMetrics()
{
    UpdateProductionMetrics();
    return CurrentMetrics;
}

float UDir_ProductionCoordinator::GetCycleProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedCount = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedCount++;
        }
    }
    
    return (float)CompletedCount / (float)AgentTasks.Num();
}

TArray<FString> UDir_ProductionCoordinator::GetBlockedAgents()
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

TArray<FString> UDir_ProductionCoordinator::GetActiveAgents()
{
    TArray<FString> ActiveAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            ActiveAgents.Add(Task.AgentName);
        }
    }
    
    return ActiveAgents;
}

void UDir_ProductionCoordinator::TriggerEmergencyStop(const FString& Reason)
{
    bProductionActive = false;
    
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY STOP TRIGGERED: %s"), *Reason);
    
    // Set all working agents to blocked
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Blocked;
        }
    }
    
    OnEmergencyTriggered(Reason);
}

void UDir_ProductionCoordinator::ResetProductionPipeline()
{
    bProductionActive = false;
    CurrentCycleID = TEXT("");
    
    // Reset all agent tasks
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.TaskDescription = FString::Printf(TEXT("Reset - Awaiting new assignment"));
        Task.Deliverables.Empty();
        Task.Dependencies.Empty();
    }
    
    CurrentMetrics = FDir_ProductionMetrics();
    CurrentMetrics.TotalAgents = AgentTasks.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline reset"));
}

bool UDir_ProductionCoordinator::IsAgentBlocked(const FString& AgentName)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    return Task && Task->Status == EDir_AgentStatus::Blocked;
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    CurrentMetrics.ActiveAgents = 0;
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.FailedTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Working:
                CurrentMetrics.ActiveAgents++;
                break;
            case EDir_AgentStatus::Completed:
                CurrentMetrics.CompletedTasks++;
                break;
            case EDir_AgentStatus::Failed:
                CurrentMetrics.FailedTasks++;
                break;
            default:
                break;
        }
    }
    
    CurrentMetrics.OverallProgress = GetCycleProgress();
    
    // Calculate cycle duration
    FTimespan CycleDuration = FDateTime::Now() - CurrentMetrics.CycleStartTime;
    CurrentMetrics.CycleDurationMinutes = CycleDuration.GetTotalMinutes();
}

void UDir_ProductionCoordinator::CheckForTimeouts()
{
    FDateTime CurrentTime = FDateTime::Now();
    
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working && CurrentTime > Task.DeadlineTime)
        {
            Task.Status = EDir_AgentStatus::Failed;
            UE_LOG(LogTemp, Error, TEXT("Agent %s timed out"), *Task.AgentName);
        }
    }
}

void UDir_ProductionCoordinator::ValidateAgentChain()
{
    // Check for circular dependencies and broken chains
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            if (!CheckAgentDependencies(Task.AgentName))
            {
                // Dependencies still not met, keep blocked
                continue;
            }
            else
            {
                // Dependencies resolved, unblock
                const_cast<FDir_AgentTask&>(Task).Status = EDir_AgentStatus::Idle;
                UE_LOG(LogTemp, Log, TEXT("Agent %s unblocked - dependencies resolved"), 
                       *Task.AgentName);
            }
        }
    }
}

FDir_AgentTask* UDir_ProductionCoordinator::FindAgentTask(const FString& AgentName)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            return &Task;
        }
    }
    return nullptr;
}