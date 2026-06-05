#include "Dir_ProductionCoordinator.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    TotalEstimatedHours = 0.0f;
    TotalCompletedHours = 0.0f;
    LastUpdateTime = FDateTime::Now();
}

void UDir_ProductionCoordinator::InitializeAgents()
{
    AgentTasks.Empty();
    
    // Initialize all 19 agents with their specializations
    TArray<FString> AgentNames = {
        "Engine Architect",
        "Core Systems Programmer", 
        "Performance Optimizer",
        "Procedural World Generator",
        "Environment Artist",
        "Architecture & Interior Agent",
        "Lighting & Atmosphere Agent",
        "Character Artist Agent",
        "Animation Agent",
        "NPC Behavior Agent",
        "Combat & Enemy AI Agent",
        "Crowd & Traffic Simulation",
        "Quest & Mission Designer",
        "Narrative & Dialogue Agent",
        "Audio Agent",
        "VFX Agent",
        "QA & Testing Agent",
        "Integration & Build Agent"
    };

    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = i + 2; // Agent #1 is Studio Director
        NewTask.AgentName = AgentNames[i];
        NewTask.TaskDescription = "Awaiting assignment";
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = 1.0f;
        NewTask.EstimatedHours = 1.0f;
        NewTask.ActualHours = 0.0f;
        
        AgentTasks.Add(NewTask);
    }

    InitializeDefaultMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized with %d agents"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::InitializeDefaultMilestones()
{
    Milestones.Empty();

    // Milestone 1: Walk Around (Agents #2, #3, #5, #9, #10)
    FDir_ProductionMilestone WalkAroundMilestone;
    WalkAroundMilestone.Type = EDir_MilestoneType::WalkAround;
    WalkAroundMilestone.Name = "Walk Around Prototype";
    WalkAroundMilestone.Description = "Player can walk, run, jump on terrain with basic landscape";
    WalkAroundMilestone.RequiredAgents = {2, 3, 5, 9, 10}; // Engine, Core, World, Character, Animation
    WalkAroundMilestone.bIsCompleted = false;
    WalkAroundMilestone.CompletionPercentage = 0.0f;
    WalkAroundMilestone.Deadline = FDateTime::Now() + FTimespan::FromDays(2);
    Milestones.Add(WalkAroundMilestone);

    // Milestone 2: Basic Survival (Agents #11, #14, #15, #16)
    FDir_ProductionMilestone SurvivalMilestone;
    SurvivalMilestone.Type = EDir_MilestoneType::BasicSurvival;
    SurvivalMilestone.Name = "Basic Survival Systems";
    SurvivalMilestone.Description = "Health, hunger, thirst, stamina systems with UI";
    SurvivalMilestone.RequiredAgents = {11, 14, 15, 16}; // NPC, Quest, Narrative, Audio
    SurvivalMilestone.bIsCompleted = false;
    SurvivalMilestone.CompletionPercentage = 0.0f;
    SurvivalMilestone.Deadline = FDateTime::Now() + FTimespan::FromDays(3);
    Milestones.Add(SurvivalMilestone);

    // Milestone 3: Dinosaur Encounters (Agents #6, #7, #8, #12, #17)
    FDir_ProductionMilestone DinosaurMilestone;
    DinosaurMilestone.Type = EDir_MilestoneType::DinosaurEncounters;
    DinosaurMilestone.Name = "Dinosaur Encounters";
    DinosaurMilestone.Description = "3-5 dinosaur species with basic AI and visual effects";
    DinosaurMilestone.RequiredAgents = {6, 7, 8, 12, 17}; // Environment, Architecture, Lighting, Combat AI, VFX
    DinosaurMilestone.bIsCompleted = false;
    DinosaurMilestone.CompletionPercentage = 0.0f;
    DinosaurMilestone.Deadline = FDateTime::Now() + FTimespan::FromDays(5);
    Milestones.Add(DinosaurMilestone);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d production milestones"), Milestones.Num());
}

void UDir_ProductionCoordinator::AssignTask(int32 AgentID, const FString& TaskDescription, float Priority)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.TaskDescription = TaskDescription;
            Task.Priority = Priority;
            Task.Status = EDir_AgentStatus::Working;
            
            UE_LOG(LogTemp, Warning, TEXT("Assigned task to Agent #%d: %s"), AgentID, *TaskDescription);
            return;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Agent #%d not found for task assignment"), AgentID);
}

void UDir_ProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            EDir_AgentStatus OldStatus = Task.Status;
            Task.Status = NewStatus;
            
            if (NewStatus == EDir_AgentStatus::Completed)
            {
                Task.ActualHours = Task.EstimatedHours; // Simplified completion tracking
                TotalCompletedHours += Task.EstimatedHours;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d status: %s -> %s"), 
                AgentID, 
                *UEnum::GetValueAsString(OldStatus),
                *UEnum::GetValueAsString(NewStatus));
            
            UpdateMilestoneProgress(EDir_MilestoneType::WalkAround); // Update all milestones
            return;
        }
    }
}

bool UDir_ProductionCoordinator::CanAgentStart(int32 AgentID) const
{
    const FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& T) {
        return T.AgentID == AgentID;
    });
    
    if (!Task)
    {
        return false;
    }
    
    return CheckDependenciesComplete(Task->Dependencies);
}

bool UDir_ProductionCoordinator::CheckDependenciesComplete(const TArray<int32>& Dependencies) const
{
    for (int32 DepAgentID : Dependencies)
    {
        const FDir_AgentTask* DepTask = AgentTasks.FindByPredicate([DepAgentID](const FDir_AgentTask& T) {
            return T.AgentID == DepAgentID;
        });
        
        if (!DepTask || DepTask->Status != EDir_AgentStatus::Completed)
        {
            return false;
        }
    }
    
    return true;
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working || Task.Status == EDir_AgentStatus::Blocked)
        {
            PendingTasks.Add(Task);
        }
    }
    
    // Sort by priority
    PendingTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    return PendingTasks;
}

FDir_AgentTask UDir_ProductionCoordinator::GetAgentTask(int32 AgentID) const
{
    const FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& T) {
        return T.AgentID == AgentID;
    });
    
    return Task ? *Task : FDir_AgentTask();
}

void UDir_ProductionCoordinator::CreateMilestone(EDir_MilestoneType Type, const FString& Name, const TArray<int32>& RequiredAgents)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.Type = Type;
    NewMilestone.Name = Name;
    NewMilestone.RequiredAgents = RequiredAgents;
    NewMilestone.bIsCompleted = false;
    NewMilestone.CompletionPercentage = 0.0f;
    NewMilestone.Deadline = FDateTime::Now() + FTimespan::FromDays(7);
    
    Milestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("Created milestone: %s"), *Name);
}

void UDir_ProductionCoordinator::UpdateMilestoneProgress(EDir_MilestoneType Type)
{
    for (FDir_ProductionMilestone& Milestone : Milestones)
    {
        if (Milestone.Type == Type || Type == EDir_MilestoneType::WalkAround) // Update all if WalkAround
        {
            int32 CompletedAgents = 0;
            int32 TotalAgents = Milestone.RequiredAgents.Num();
            
            for (int32 AgentID : Milestone.RequiredAgents)
            {
                const FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& T) {
                    return T.AgentID == AgentID;
                });
                
                if (Task && Task->Status == EDir_AgentStatus::Completed)
                {
                    CompletedAgents++;
                }
            }
            
            float NewPercentage = TotalAgents > 0 ? (float)CompletedAgents / TotalAgents * 100.0f : 0.0f;
            Milestone.CompletionPercentage = NewPercentage;
            Milestone.bIsCompleted = (NewPercentage >= 100.0f);
            
            UE_LOG(LogTemp, Warning, TEXT("Milestone '%s' progress: %.1f%% (%d/%d agents)"), 
                *Milestone.Name, NewPercentage, CompletedAgents, TotalAgents);
        }
    }
}

float UDir_ProductionCoordinator::GetMilestoneCompletion(EDir_MilestoneType Type) const
{
    const FDir_ProductionMilestone* Milestone = Milestones.FindByPredicate([Type](const FDir_ProductionMilestone& M) {
        return M.Type == Type;
    });
    
    return Milestone ? Milestone->CompletionPercentage : 0.0f;
}

TArray<FDir_ProductionMilestone> UDir_ProductionCoordinator::GetActiveMilestones() const
{
    TArray<FDir_ProductionMilestone> ActiveMilestones;
    
    for (const FDir_ProductionMilestone& Milestone : Milestones)
    {
        if (!Milestone.bIsCompleted)
        {
            ActiveMilestones.Add(Milestone);
        }
    }
    
    return ActiveMilestones;
}

float UDir_ProductionCoordinator::GetOverallProgress() const
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    return (float)CompletedTasks / AgentTasks.Num() * 100.0f;
}

int32 UDir_ProductionCoordinator::GetActiveAgentCount() const
{
    int32 ActiveCount = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

int32 UDir_ProductionCoordinator::GetBlockedAgentCount() const
{
    int32 BlockedCount = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedCount++;
        }
    }
    return BlockedCount;
}

FString UDir_ProductionCoordinator::GenerateProductionReport() const
{
    FString Report = TEXT("=== PRODUCTION REPORT ===\n");
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), GetOverallProgress());
    Report += FString::Printf(TEXT("Active Agents: %d\n"), GetActiveAgentCount());
    Report += FString::Printf(TEXT("Blocked Agents: %d\n"), GetBlockedAgentCount());
    Report += FString::Printf(TEXT("Total Hours: %.1f / %.1f\n"), TotalCompletedHours, TotalEstimatedHours);
    
    Report += TEXT("\n=== MILESTONES ===\n");
    for (const FDir_ProductionMilestone& Milestone : Milestones)
    {
        Report += FString::Printf(TEXT("%s: %.1f%% %s\n"), 
            *Milestone.Name, 
            Milestone.CompletionPercentage,
            Milestone.bIsCompleted ? TEXT("[COMPLETE]") : TEXT("[IN PROGRESS]"));
    }
    
    Report += TEXT("\n=== AGENT STATUS ===\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        Report += FString::Printf(TEXT("Agent #%d (%s): %s - %s\n"), 
            Task.AgentID, 
            *Task.AgentName, 
            *UEnum::GetValueAsString(Task.Status),
            *Task.TaskDescription);
    }
    
    return Report;
}

TArray<int32> UDir_ProductionCoordinator::GetCriticalPath() const
{
    // Simplified critical path - agents with highest priority and dependencies
    TArray<int32> CriticalAgents = {2, 3, 5, 9, 10}; // Core systems for Walk Around milestone
    return CriticalAgents;
}

void UDir_ProductionCoordinator::OptimizeTaskSchedule()
{
    // Sort tasks by priority and dependencies
    AgentTasks.Sort([this](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        // Higher priority first
        if (A.Priority != B.Priority)
        {
            return A.Priority > B.Priority;
        }
        
        // Fewer dependencies first
        return A.Dependencies.Num() < B.Dependencies.Num();
    });
    
    UE_LOG(LogTemp, Warning, TEXT("Task schedule optimized"));
}

bool UDir_ProductionCoordinator::ValidateAgentOutput(int32 AgentID, const FString& OutputPath)
{
    // Check if output files exist
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    if (PlatformFile.FileExists(*OutputPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d output validated: %s"), AgentID, *OutputPath);
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Agent #%d output missing: %s"), AgentID, *OutputPath);
    return false;
}

void UDir_ProductionCoordinator::TriggerQualityCheck()
{
    UE_LOG(LogTemp, Warning, TEXT("Quality check triggered - validating all agent outputs"));
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed && !Task.OutputFiles.IsEmpty())
        {
            ValidateAgentOutput(Task.AgentID, Task.OutputFiles);
        }
    }
}