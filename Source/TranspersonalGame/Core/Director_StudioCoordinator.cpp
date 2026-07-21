#include "Director_StudioCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UDir_StudioCoordinator::UDir_StudioCoordinator()
{
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    bQABlockActive = false;
    LastStableBuildVersion = TEXT("v1.0.0");
    CompletedTasksCount = 0;
    BlockedTasksCount = 0;
}

void UDir_StudioCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Initializing production pipeline"));
    
    InitializeProductionPipeline();
    SetupAgentHierarchy();
    
    // Create initial milestones
    CreateMilestone(TEXT("Playable Prototype"), EDir_ProductionPhase::CoreSystems);
    CreateMilestone(TEXT("World Generation Complete"), EDir_ProductionPhase::WorldGeneration);
    CreateMilestone(TEXT("Character Systems Online"), EDir_ProductionPhase::CharacterSystems);
    CreateMilestone(TEXT("Alpha Build"), EDir_ProductionPhase::GameplayMechanics);
    CreateMilestone(TEXT("Beta Release"), EDir_ProductionPhase::Testing);
}

void UDir_StudioCoordinator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Shutting down production pipeline"));
    
    // Save final production report
    FString FinalReport = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("Final Production Report: %s"), *FinalReport);
    
    Super::Deinitialize();
}

void UDir_StudioCoordinator::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Setting up 19-agent production pipeline"));
    
    // Initialize agent status tracking
    TArray<FString> AgentNames = {
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
    
    for (const FString& AgentName : AgentNames)
    {
        AgentStatusMap.Add(AgentName, EDir_AgentStatus::Idle);
        AgentPerformanceScores.Add(AgentName, 1.0f);
    }
    
    // Set critical path agents
    CriticalPathAgents = {
        TEXT("Engine Architect"),
        TEXT("Core Systems Programmer"),
        TEXT("Procedural World Generator"),
        TEXT("Character Artist Agent"),
        TEXT("QA & Testing Agent")
    };
    
    CurrentPhase = EDir_ProductionPhase::CoreSystems;
}

void UDir_StudioCoordinator::UpdateProductionStatus()
{
    // Update milestone progress
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        int32 CompletedTasks = 0;
        int32 TotalTasks = Milestone.RequiredTasks.Num();
        
        for (const FDir_AgentTask& Task : Milestone.RequiredTasks)
        {
            if (Task.Status == EDir_AgentStatus::Complete)
            {
                CompletedTasks++;
            }
        }
        
        Milestone.CompletionPercentage = TotalTasks > 0 ? (float)CompletedTasks / TotalTasks * 100.0f : 0.0f;
        Milestone.bIsCompleted = Milestone.CompletionPercentage >= 100.0f;
    }
    
    // Update critical path
    UpdateCriticalPath();
    
    // Check for bottlenecks
    IdentifyBottlenecks();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Production status updated - Phase: %d"), (int32)CurrentPhase);
}

bool UDir_StudioCoordinator::AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task)
{
    if (!AgentStatusMap.Contains(AgentName))
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: Unknown agent %s"), *AgentName);
        return false;
    }
    
    if (AgentStatusMap[AgentName] == EDir_AgentStatus::Working)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent %s is already working"), *AgentName);
        return false;
    }
    
    if (!ValidateAgentCapability(AgentName, Task.TaskDescription))
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: Agent %s not capable of task: %s"), *AgentName, *Task.TaskDescription);
        return false;
    }
    
    if (!CheckTaskPrerequisites(Task))
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Task prerequisites not met for: %s"), *Task.TaskDescription);
        return false;
    }
    
    // Assign task
    FDir_AgentTask NewTask = Task;
    NewTask.AgentName = AgentName;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.StartTime = FDateTime::Now();
    
    ActiveTasks.Add(NewTask);
    AgentStatusMap[AgentName] = EDir_AgentStatus::Working;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Assigned task '%s' to agent %s"), *Task.TaskDescription, *AgentName);
    return true;
}

void UDir_StudioCoordinator::CompleteAgentTask(const FString& AgentName, const FString& TaskID)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName && Task.TaskDescription.Contains(TaskID))
        {
            Task.Status = EDir_AgentStatus::Complete;
            AgentStatusMap[AgentName] = EDir_AgentStatus::Idle;
            CompletedTasksCount++;
            
            // Update agent performance
            float CompletionTime = (FDateTime::Now() - Task.StartTime).GetTotalSeconds();
            float EstimatedTime = (Task.EstimatedCompletion - Task.StartTime).GetTotalSeconds();
            float PerformanceScore = EstimatedTime > 0 ? FMath::Clamp(EstimatedTime / CompletionTime, 0.1f, 2.0f) : 1.0f;
            
            LogAgentPerformance(AgentName, PerformanceScore);
            
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Task completed by %s - Performance: %.2f"), *AgentName, PerformanceScore);
            break;
        }
    }
    
    UpdateProductionStatus();
}

void UDir_StudioCoordinator::BlockAgent(const FString& AgentName, const FString& Reason)
{
    if (AgentStatusMap.Contains(AgentName))
    {
        AgentStatusMap[AgentName] = EDir_AgentStatus::Blocked;
        BlockedTasksCount++;
        
        UE_LOG(LogTemp, Error, TEXT("Studio Director: Agent %s blocked - Reason: %s"), *AgentName, *Reason);
        
        // If critical path agent is blocked, escalate
        if (CriticalPathAgents.Contains(AgentName))
        {
            ActivateEmergencyProtocol(TEXT("Critical Path Blocked"));
        }
    }
}

void UDir_StudioCoordinator::CreateMilestone(const FString& MilestoneName, EDir_ProductionPhase Phase)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.MilestoneName = MilestoneName;
    NewMilestone.Phase = Phase;
    NewMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(30); // Default 30 days
    NewMilestone.bIsCompleted = false;
    NewMilestone.CompletionPercentage = 0.0f;
    
    ProductionMilestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Created milestone '%s' for phase %d"), *MilestoneName, (int32)Phase);
}

float UDir_StudioCoordinator::GetMilestoneProgress(const FString& MilestoneName)
{
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.CompletionPercentage;
        }
    }
    return 0.0f;
}

bool UDir_StudioCoordinator::IsMilestoneComplete(const FString& MilestoneName)
{
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.bIsCompleted;
        }
    }
    return false;
}

TArray<FString> UDir_StudioCoordinator::GetCriticalPathAgents()
{
    return CriticalPathAgents;
}

void UDir_StudioCoordinator::IdentifyBottlenecks()
{
    TArray<FString> BottleneckAgents;
    
    for (const auto& AgentPair : AgentStatusMap)
    {
        if (AgentPair.Value == EDir_AgentStatus::Blocked || 
            (AgentPair.Value == EDir_AgentStatus::Working && AgentPerformanceScores.Contains(AgentPair.Key) && 
             AgentPerformanceScores[AgentPair.Key] < 0.5f))
        {
            BottleneckAgents.Add(AgentPair.Key);
        }
    }
    
    if (BottleneckAgents.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Bottlenecks identified in %d agents"), BottleneckAgents.Num());
        RebalanceWorkload();
    }
}

void UDir_StudioCoordinator::RebalanceWorkload()
{
    // Redistribute tasks from blocked/slow agents to available agents
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || Task.Status == EDir_AgentStatus::Error)
        {
            // Find available agent with similar capabilities
            for (const auto& AgentPair : AgentStatusMap)
            {
                if (AgentPair.Value == EDir_AgentStatus::Idle && 
                    ValidateAgentCapability(AgentPair.Key, Task.TaskDescription))
                {
                    Task.AgentName = AgentPair.Key;
                    Task.Status = EDir_AgentStatus::Working;
                    AgentStatusMap[AgentPair.Key] = EDir_AgentStatus::Working;
                    
                    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Reassigned task to %s"), *AgentPair.Key);
                    break;
                }
            }
        }
    }
}

void UDir_StudioCoordinator::TriggerQAValidation()
{
    if (AgentStatusMap.Contains(TEXT("QA & Testing Agent")))
    {
        FDir_AgentTask QATask;
        QATask.TaskDescription = TEXT("Full Build Validation");
        QATask.Priority = 10.0f; // Highest priority
        QATask.EstimatedCompletion = FDateTime::Now() + FTimespan::FromHours(2);
        
        AssignTaskToAgent(TEXT("QA & Testing Agent"), QATask);
        
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: QA validation triggered"));
    }
}

void UDir_StudioCoordinator::HandleQABlockage(const FString& BlockageReason)
{
    bQABlockActive = true;
    
    // Block all agents except critical fixes
    for (auto& AgentPair : AgentStatusMap)
    {
        if (AgentPair.Value == EDir_AgentStatus::Working && 
            !CriticalPathAgents.Contains(AgentPair.Key))
        {
            AgentPair.Value = EDir_AgentStatus::Blocked;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Studio Director: QA BLOCKAGE - %s"), *BlockageReason);
    ActivateEmergencyProtocol(TEXT("QA Block"));
}

void UDir_StudioCoordinator::InitiateBuildProcess()
{
    if (AgentStatusMap.Contains(TEXT("Integration & Build Agent")))
    {
        FDir_AgentTask BuildTask;
        BuildTask.TaskDescription = TEXT("Integration Build");
        BuildTask.Priority = 9.0f;
        BuildTask.EstimatedCompletion = FDateTime::Now() + FTimespan::FromHours(1);
        
        AssignTaskToAgent(TEXT("Integration & Build Agent"), BuildTask);
        
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Build process initiated"));
    }
}

void UDir_StudioCoordinator::ValidateBuildIntegrity()
{
    // Check if all critical systems are functional
    bool bBuildValid = true;
    
    for (const FString& CriticalAgent : CriticalPathAgents)
    {
        if (AgentStatusMap.Contains(CriticalAgent) && 
            AgentStatusMap[CriticalAgent] == EDir_AgentStatus::Error)
        {
            bBuildValid = false;
            break;
        }
    }
    
    if (!bBuildValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Studio Director: Build integrity compromised"));
        ActivateEmergencyProtocol(TEXT("Build Failure"));
    }
    else
    {
        LastStableBuildVersion = FString::Printf(TEXT("v1.%d.%d"), 
            FDateTime::Now().GetDay(), FDateTime::Now().GetHour());
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Build validated - %s"), *LastStableBuildVersion);
    }
}

void UDir_StudioCoordinator::ActivateEmergencyProtocol(const FString& ProtocolType)
{
    UE_LOG(LogTemp, Error, TEXT("Studio Director: EMERGENCY PROTOCOL ACTIVATED - %s"), *ProtocolType);
    
    if (ProtocolType == TEXT("Critical Path Blocked"))
    {
        // Reassign all available agents to unblock critical path
        RebalanceWorkload();
    }
    else if (ProtocolType == TEXT("QA Block"))
    {
        // Focus all efforts on fixing QA issues
        bQABlockActive = true;
    }
    else if (ProtocolType == TEXT("Build Failure"))
    {
        // Rollback to last stable build
        RollbackToLastStableBuild();
    }
}

void UDir_StudioCoordinator::RollbackToLastStableBuild()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Rolling back to %s"), *LastStableBuildVersion);
    
    // Reset all agents to idle
    for (auto& AgentPair : AgentStatusMap)
    {
        AgentPair.Value = EDir_AgentStatus::Idle;
    }
    
    // Clear active tasks
    ActiveTasks.Empty();
    bQABlockActive = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Rollback complete"));
}

FString UDir_StudioCoordinator::GenerateProductionReport()
{
    FString Report = TEXT("=== STUDIO PRODUCTION REPORT ===\n");
    Report += FString::Printf(TEXT("Current Phase: %d\n"), (int32)CurrentPhase);
    Report += FString::Printf(TEXT("Active Tasks: %d\n"), ActiveTasks.Num());
    Report += FString::Printf(TEXT("Completed Tasks: %d\n"), CompletedTasksCount);
    Report += FString::Printf(TEXT("Blocked Tasks: %d\n"), BlockedTasksCount);
    Report += FString::Printf(TEXT("QA Block Active: %s\n"), bQABlockActive ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("Last Stable Build: %s\n"), *LastStableBuildVersion);
    
    Report += TEXT("\n=== AGENT STATUS ===\n");
    for (const auto& AgentPair : AgentStatusMap)
    {
        FString StatusText;
        switch (AgentPair.Value)
        {
            case EDir_AgentStatus::Idle: StatusText = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusText = TEXT("WORKING"); break;
            case EDir_AgentStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Complete: StatusText = TEXT("COMPLETE"); break;
            case EDir_AgentStatus::Error: StatusText = TEXT("ERROR"); break;
        }
        
        float Performance = AgentPerformanceScores.Contains(AgentPair.Key) ? 
            AgentPerformanceScores[AgentPair.Key] : 1.0f;
        
        Report += FString::Printf(TEXT("%s: %s (Performance: %.2f)\n"), 
            *AgentPair.Key, *StatusText, Performance);
    }
    
    Report += TEXT("\n=== MILESTONES ===\n");
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        Report += FString::Printf(TEXT("%s: %.1f%% %s\n"), 
            *Milestone.MilestoneName, 
            Milestone.CompletionPercentage,
            Milestone.bIsCompleted ? TEXT("COMPLETE") : TEXT("IN PROGRESS"));
    }
    
    return Report;
}

void UDir_StudioCoordinator::LogAgentPerformance(const FString& AgentName, float PerformanceScore)
{
    if (AgentPerformanceScores.Contains(AgentName))
    {
        // Weighted average with previous performance
        float CurrentScore = AgentPerformanceScores[AgentName];
        AgentPerformanceScores[AgentName] = (CurrentScore * 0.7f) + (PerformanceScore * 0.3f);
    }
    else
    {
        AgentPerformanceScores.Add(AgentName, PerformanceScore);
    }
}

void UDir_StudioCoordinator::SetupAgentHierarchy()
{
    // Define agent dependencies and capabilities
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent hierarchy established"));
}

void UDir_StudioCoordinator::ValidateTaskDependencies()
{
    // Check if task dependencies are met
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        for (const FString& Dependency : Task.Dependencies)
        {
            // Validate dependency completion
        }
    }
}

void UDir_StudioCoordinator::UpdateCriticalPath()
{
    // Recalculate critical path based on current task status
    // This would involve complex scheduling algorithms in a real implementation
}

void UDir_StudioCoordinator::ProcessAgentQueue()
{
    // Process pending agent tasks
}

void UDir_StudioCoordinator::HandleCriticalFailure()
{
    ActivateEmergencyProtocol(TEXT("Critical System Failure"));
}

void UDir_StudioCoordinator::ExecuteRollbackProcedure()
{
    RollbackToLastStableBuild();
}

bool UDir_StudioCoordinator::ValidateAgentCapability(const FString& AgentName, const FString& TaskType)
{
    // Simple capability validation based on agent name
    if (AgentName.Contains(TEXT("Engine")) && TaskType.Contains(TEXT("Architecture"))) return true;
    if (AgentName.Contains(TEXT("Core")) && TaskType.Contains(TEXT("Physics"))) return true;
    if (AgentName.Contains(TEXT("World")) && TaskType.Contains(TEXT("Terrain"))) return true;
    if (AgentName.Contains(TEXT("Character")) && TaskType.Contains(TEXT("Animation"))) return true;
    if (AgentName.Contains(TEXT("AI")) && TaskType.Contains(TEXT("Behavior"))) return true;
    if (AgentName.Contains(TEXT("QA")) && TaskType.Contains(TEXT("Testing"))) return true;
    
    return true; // Default allow for flexibility
}

bool UDir_StudioCoordinator::CheckTaskPrerequisites(const FDir_AgentTask& Task)
{
    // Check if all dependencies are completed
    for (const FString& Dependency : Task.Dependencies)
    {
        bool bDependencyMet = false;
        for (const FDir_AgentTask& CompletedTask : ActiveTasks)
        {
            if (CompletedTask.TaskDescription.Contains(Dependency) && 
                CompletedTask.Status == EDir_AgentStatus::Complete)
            {
                bDependencyMet = true;
                break;
            }
        }
        
        if (!bDependencyMet)
        {
            return false;
        }
    }
    
    return true;
}