#include "Dir_ProductionManager.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"

UDir_ProductionManager::UDir_ProductionManager()
{
    InitializeAgentNames();
    CurrentMetrics = FDir_ProductionMetrics();
}

void UDir_ProductionManager::InitializeAgentNames()
{
    AgentNames = {
        TEXT("Studio_Director"),
        TEXT("Engine_Architect"),
        TEXT("Core_Systems_Programmer"),
        TEXT("Performance_Optimizer"),
        TEXT("Procedural_World_Generator"),
        TEXT("Environment_Artist"),
        TEXT("Architecture_Interior_Agent"),
        TEXT("Lighting_Atmosphere_Agent"),
        TEXT("Character_Artist_Agent"),
        TEXT("Animation_Agent"),
        TEXT("NPC_Behavior_Agent"),
        TEXT("Combat_Enemy_AI_Agent"),
        TEXT("Crowd_Traffic_Simulation"),
        TEXT("Quest_Mission_Designer"),
        TEXT("Narrative_Dialogue_Agent"),
        TEXT("Audio_Agent"),
        TEXT("VFX_Agent"),
        TEXT("QA_Testing_Agent"),
        TEXT("Integration_Build_Agent")
    };
}

void UDir_ProductionManager::InitializeAgentPipeline()
{
    AgentTasks.Empty();
    
    // Initialize all agents with default idle tasks
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentName;
        NewTask.TaskDescription = TEXT("Awaiting assignment");
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = 1.0f;
        NewTask.StartTime = FDateTime::Now();
        NewTask.EstimatedCompletion = FDateTime::Now() + FTimespan::FromHours(1);
        
        AgentTasks.Add(NewTask);
    }

    // Set up initial dependencies based on production pipeline
    AddTaskDependency(TEXT("Core_Systems_Programmer"), TEXT("Engine_Architect"));
    AddTaskDependency(TEXT("Performance_Optimizer"), TEXT("Core_Systems_Programmer"));
    AddTaskDependency(TEXT("Environment_Artist"), TEXT("Procedural_World_Generator"));
    AddTaskDependency(TEXT("Architecture_Interior_Agent"), TEXT("Environment_Artist"));
    AddTaskDependency(TEXT("Lighting_Atmosphere_Agent"), TEXT("Architecture_Interior_Agent"));
    AddTaskDependency(TEXT("Animation_Agent"), TEXT("Character_Artist_Agent"));
    AddTaskDependency(TEXT("NPC_Behavior_Agent"), TEXT("Animation_Agent"));
    AddTaskDependency(TEXT("Combat_Enemy_AI_Agent"), TEXT("NPC_Behavior_Agent"));
    AddTaskDependency(TEXT("Crowd_Traffic_Simulation"), TEXT("Combat_Enemy_AI_Agent"));
    AddTaskDependency(TEXT("Quest_Mission_Designer"), TEXT("Narrative_Dialogue_Agent"));
    AddTaskDependency(TEXT("Audio_Agent"), TEXT("Quest_Mission_Designer"));
    AddTaskDependency(TEXT("VFX_Agent"), TEXT("Audio_Agent"));
    AddTaskDependency(TEXT("QA_Testing_Agent"), TEXT("VFX_Agent"));
    AddTaskDependency(TEXT("Integration_Build_Agent"), TEXT("QA_Testing_Agent"));

    // Set up production milestones
    SetMilestone(TEXT("Core_Architecture_Complete"), {TEXT("Engine_Architect"), TEXT("Core_Systems_Programmer")});
    SetMilestone(TEXT("World_Foundation_Ready"), {TEXT("Procedural_World_Generator"), TEXT("Environment_Artist"), TEXT("Performance_Optimizer")});
    SetMilestone(TEXT("Character_Systems_Online"), {TEXT("Character_Artist_Agent"), TEXT("Animation_Agent"), TEXT("NPC_Behavior_Agent")});
    SetMilestone(TEXT("Gameplay_Core_Complete"), {TEXT("Combat_Enemy_AI_Agent"), TEXT("Quest_Mission_Designer"), TEXT("Narrative_Dialogue_Agent")});
    SetMilestone(TEXT("Polish_Phase_Ready"), {TEXT("Audio_Agent"), TEXT("VFX_Agent"), TEXT("Lighting_Atmosphere_Agent")});
    SetMilestone(TEXT("Release_Candidate"), {TEXT("QA_Testing_Agent"), TEXT("Integration_Build_Agent")});

    CurrentMetrics.TotalAgents = AgentNames.Num();
    CurrentMetrics.CurrentPhase = EDir_ProductionPhase::PreProduction;

    UE_LOG(LogTemp, Warning, TEXT("Production pipeline initialized with %d agents"), AgentNames.Num());
}

void UDir_ProductionManager::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        Task->TaskDescription = TaskDescription;
        Task->Priority = Priority;
        Task->Status = EDir_AgentStatus::Working;
        Task->StartTime = FDateTime::Now();
        Task->EstimatedCompletion = FDateTime::Now() + FTimespan::FromHours(FMath::RandRange(1, 4));
        
        UE_LOG(LogTemp, Log, TEXT("Assigned task to %s: %s (Priority: %.1f)"), *AgentName, *TaskDescription, Priority);
    }
}

void UDir_ProductionManager::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        Task->Status = NewStatus;
        
        if (NewStatus == EDir_AgentStatus::Completed)
        {
            CurrentMetrics.CompletedTasks++;
        }
        else if (NewStatus == EDir_AgentStatus::Blocked)
        {
            CurrentMetrics.BlockedTasks++;
        }
    }
}

bool UDir_ProductionManager::CanAgentStartTask(const FString& AgentName) const
{
    // Check if dependencies are met
    if (!AreDependenciesMet(AgentName))
    {
        return false;
    }

    // Check if agent is not already working or blocked
    const FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        return Task->Status == EDir_AgentStatus::Idle || Task->Status == EDir_AgentStatus::Completed;
    }

    return false;
}

TArray<FString> UDir_ProductionManager::GetBlockedAgents() const
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

void UDir_ProductionManager::AdvanceProductionPhase()
{
    // Check milestones to determine phase advancement
    if (IsMilestoneComplete(TEXT("Core_Architecture_Complete")) && CurrentMetrics.CurrentPhase == EDir_ProductionPhase::PreProduction)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::CoreSystems;
    }
    else if (IsMilestoneComplete(TEXT("World_Foundation_Ready")) && CurrentMetrics.CurrentPhase == EDir_ProductionPhase::CoreSystems)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::WorldBuilding;
    }
    else if (IsMilestoneComplete(TEXT("Character_Systems_Online")) && CurrentMetrics.CurrentPhase == EDir_ProductionPhase::WorldBuilding)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::CharacterDevelopment;
    }
    else if (IsMilestoneComplete(TEXT("Gameplay_Core_Complete")) && CurrentMetrics.CurrentPhase == EDir_ProductionPhase::CharacterDevelopment)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::GameplayImplementation;
    }
    else if (IsMilestoneComplete(TEXT("Polish_Phase_Ready")) && CurrentMetrics.CurrentPhase == EDir_ProductionPhase::GameplayImplementation)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Polish;
    }
    else if (IsMilestoneComplete(TEXT("Release_Candidate")) && CurrentMetrics.CurrentPhase == EDir_ProductionPhase::Polish)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Testing;
    }
}

FDir_ProductionMetrics UDir_ProductionManager::GetProductionMetrics() const
{
    FDir_ProductionMetrics Metrics = CurrentMetrics;
    
    // Update real-time metrics
    Metrics.ActiveAgents = 0;
    Metrics.CompletedTasks = 0;
    Metrics.BlockedTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Metrics.ActiveAgents++;
        }
        else if (Task.Status == EDir_AgentStatus::Completed)
        {
            Metrics.CompletedTasks++;
        }
        else if (Task.Status == EDir_AgentStatus::Blocked)
        {
            Metrics.BlockedTasks++;
        }
    }
    
    Metrics.OverallProgress = CalculateOverallProgress();
    
    return Metrics;
}

float UDir_ProductionManager::CalculateOverallProgress() const
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
    
    return (float)CompletedCount / (float)AgentTasks.Num() * 100.0f;
}

void UDir_ProductionManager::AddTaskDependency(const FString& AgentName, const FString& DependentAgent)
{
    if (!TaskDependencies.Contains(AgentName))
    {
        TaskDependencies.Add(AgentName, TArray<FString>());
    }
    
    TaskDependencies[AgentName].AddUnique(DependentAgent);
}

bool UDir_ProductionManager::AreDependenciesMet(const FString& AgentName) const
{
    if (!TaskDependencies.Contains(AgentName))
    {
        return true; // No dependencies
    }
    
    const TArray<FString>& Dependencies = TaskDependencies[AgentName];
    
    for (const FString& Dependency : Dependencies)
    {
        const FDir_AgentTask* DependentTask = FindAgentTask(Dependency);
        if (!DependentTask || DependentTask->Status != EDir_AgentStatus::Completed)
        {
            return false;
        }
    }
    
    return true;
}

void UDir_ProductionManager::SetMilestone(const FString& MilestoneName, const TArray<FString>& RequiredAgents)
{
    ProductionMilestones.Add(MilestoneName, RequiredAgents);
}

bool UDir_ProductionManager::IsMilestoneComplete(const FString& MilestoneName) const
{
    if (!ProductionMilestones.Contains(MilestoneName))
    {
        return false;
    }
    
    const TArray<FString>& RequiredAgents = ProductionMilestones[MilestoneName];
    
    for (const FString& AgentName : RequiredAgents)
    {
        const FDir_AgentTask* Task = FindAgentTask(AgentName);
        if (!Task || Task->Status != EDir_AgentStatus::Completed)
        {
            return false;
        }
    }
    
    return true;
}

void UDir_ProductionManager::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS ==="));
    
    FDir_ProductionMetrics Metrics = GetProductionMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), Metrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d/%d"), Metrics.ActiveAgents, Metrics.TotalAgents);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), Metrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked Tasks: %d"), Metrics.BlockedTasks);
    
    FString PhaseString;
    switch (Metrics.CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction: PhaseString = TEXT("Pre-Production"); break;
        case EDir_ProductionPhase::CoreSystems: PhaseString = TEXT("Core Systems"); break;
        case EDir_ProductionPhase::WorldBuilding: PhaseString = TEXT("World Building"); break;
        case EDir_ProductionPhase::CharacterDevelopment: PhaseString = TEXT("Character Development"); break;
        case EDir_ProductionPhase::GameplayImplementation: PhaseString = TEXT("Gameplay Implementation"); break;
        case EDir_ProductionPhase::Polish: PhaseString = TEXT("Polish"); break;
        case EDir_ProductionPhase::Testing: PhaseString = TEXT("Testing"); break;
    }
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %s"), *PhaseString);
    
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT DETAILS ==="));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusString;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusString = TEXT("Idle"); break;
            case EDir_AgentStatus::Working: StatusString = TEXT("Working"); break;
            case EDir_AgentStatus::Completed: StatusString = TEXT("Completed"); break;
            case EDir_AgentStatus::Blocked: StatusString = TEXT("Blocked"); break;
            case EDir_AgentStatus::Error: StatusString = TEXT("Error"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s [%s] (Priority: %.1f)"), 
               *Task.AgentName, *Task.TaskDescription, *StatusString, Task.Priority);
    }
}

void UDir_ProductionManager::ResetAllAgentTasks()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.TaskDescription = TEXT("Awaiting assignment");
        Task.StartTime = FDateTime::Now();
        Task.EstimatedCompletion = FDateTime::Now();
    }
    
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.BlockedTasks = 0;
    CurrentMetrics.ActiveAgents = 0;
    CurrentMetrics.OverallProgress = 0.0f;
    CurrentMetrics.CurrentPhase = EDir_ProductionPhase::PreProduction;
    
    UE_LOG(LogTemp, Warning, TEXT("All agent tasks reset to idle state"));
}

FDir_AgentTask* UDir_ProductionManager::FindAgentTask(const FString& AgentName)
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

const FDir_AgentTask* UDir_ProductionManager::FindAgentTask(const FString& AgentName) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            return &Task;
        }
    }
    return nullptr;
}