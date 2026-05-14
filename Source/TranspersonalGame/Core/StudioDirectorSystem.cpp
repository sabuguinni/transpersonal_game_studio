#include "StudioDirectorSystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    // Initialize default values
}

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Initialized"));
    
    // Initialize agent tasks for the production pipeline
    InitializeAgentTasks();
    
    // Set up initial milestones
    Milestones.Add(TEXT("Milestone_1_Walk_Around"), false);
    Milestones.Add(TEXT("Milestone_2_Basic_Survival"), false);
    Milestones.Add(TEXT("Milestone_3_Dinosaur_Encounters"), false);
    Milestones.Add(TEXT("Milestone_4_World_Populated"), false);
    Milestones.Add(TEXT("Milestone_5_Full_Gameplay"), false);
}

void UStudioDirectorSystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System Deinitialized"));
    Super::Deinitialize();
}

void UStudioDirectorSystem::StartProductionCycle(const FString& CycleID)
{
    if (IsProductionCycleActive())
    {
        UE_LOG(LogTemp, Warning, TEXT("Production cycle already active: %s"), *CurrentCycle.CycleID);
        return;
    }

    CurrentCycle.CycleID = CycleID;
    CurrentCycle.StartTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
    CurrentCycle.bCycleCompleted = false;
    
    // Reset all agent statuses to Idle
    for (FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Started production cycle: %s"), *CycleID);
}

void UStudioDirectorSystem::CompleteProductionCycle()
{
    if (!IsProductionCycleActive())
    {
        UE_LOG(LogTemp, Warning, TEXT("No active production cycle to complete"));
        return;
    }

    CurrentCycle.EndTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
    CurrentCycle.bCycleCompleted = true;
    
    // Archive the completed cycle
    CycleHistory.Add(CurrentCycle);
    
    UE_LOG(LogTemp, Warning, TEXT("Completed production cycle: %s (Duration: %.2f seconds)"), 
           *CurrentCycle.CycleID, CurrentCycle.EndTime - CurrentCycle.StartTime);
    
    // Reset current cycle
    CurrentCycle = FDir_ProductionCycle();
}

bool UStudioDirectorSystem::IsProductionCycleActive() const
{
    return !CurrentCycle.CycleID.IsEmpty() && !CurrentCycle.bCycleCompleted;
}

void UStudioDirectorSystem::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask* ExistingTask = FindAgentTask(AgentName);
    if (ExistingTask)
    {
        ExistingTask->TaskDescription = TaskDescription;
        ExistingTask->Priority = Priority;
        ExistingTask->Status = EDir_AgentStatus::Idle;
        UE_LOG(LogTemp, Log, TEXT("Updated task for agent %s: %s"), *AgentName, *TaskDescription);
    }
    else
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentName;
        NewTask.TaskDescription = TaskDescription;
        NewTask.Priority = Priority;
        NewTask.Status = EDir_AgentStatus::Idle;
        CurrentCycle.AgentTasks.Add(NewTask);
        UE_LOG(LogTemp, Log, TEXT("Assigned new task to agent %s: %s"), *AgentName, *TaskDescription);
    }
}

void UStudioDirectorSystem::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        Task->Status = NewStatus;
        UE_LOG(LogTemp, Log, TEXT("Updated agent %s status to %d"), *AgentName, (int32)NewStatus);
    }
}

EDir_AgentStatus UStudioDirectorSystem::GetAgentStatus(const FString& AgentName) const
{
    const FDir_AgentTask* Task = FindAgentTask(AgentName);
    return Task ? Task->Status : EDir_AgentStatus::Idle;
}

TArray<FDir_AgentTask> UStudioDirectorSystem::GetAllAgentTasks() const
{
    return CurrentCycle.AgentTasks;
}

void UStudioDirectorSystem::AddTaskDependency(const FString& AgentName, const FString& DependencyAgent)
{
    FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (Task)
    {
        Task->Dependencies.AddUnique(DependencyAgent);
        UE_LOG(LogTemp, Log, TEXT("Added dependency: %s depends on %s"), *AgentName, *DependencyAgent);
    }
}

bool UStudioDirectorSystem::CanAgentStart(const FString& AgentName) const
{
    const FDir_AgentTask* Task = FindAgentTask(AgentName);
    if (!Task)
    {
        return false;
    }

    // Check if all dependencies are completed
    for (const FString& Dependency : Task->Dependencies)
    {
        EDir_AgentStatus DepStatus = GetAgentStatus(Dependency);
        if (DepStatus != EDir_AgentStatus::Completed)
        {
            return false;
        }
    }

    return true;
}

TArray<FString> UStudioDirectorSystem::GetBlockedAgents() const
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || !CanAgentStart(Task.AgentName))
        {
            BlockedAgents.Add(Task.AgentName);
        }
    }
    
    return BlockedAgents;
}

float UStudioDirectorSystem::GetCycleProgress() const
{
    if (CurrentCycle.AgentTasks.Num() == 0)
    {
        return 0.0f;
    }

    int32 CompletedCount = GetCompletedTaskCount();
    return (float)CompletedCount / (float)CurrentCycle.AgentTasks.Num();
}

int32 UStudioDirectorSystem::GetCompletedTaskCount() const
{
    int32 Count = 0;
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            Count++;
        }
    }
    return Count;
}

int32 UStudioDirectorSystem::GetTotalTaskCount() const
{
    return CurrentCycle.AgentTasks.Num();
}

void UStudioDirectorSystem::SetMilestone(const FString& MilestoneName, bool bCompleted)
{
    Milestones.Add(MilestoneName, bCompleted);
    UE_LOG(LogTemp, Warning, TEXT("Milestone %s set to %s"), *MilestoneName, bCompleted ? TEXT("COMPLETED") : TEXT("INCOMPLETE"));
}

bool UStudioDirectorSystem::IsMilestoneCompleted(const FString& MilestoneName) const
{
    const bool* bCompleted = Milestones.Find(MilestoneName);
    return bCompleted ? *bCompleted : false;
}

TArray<FString> UStudioDirectorSystem::GetCompletedMilestones() const
{
    TArray<FString> CompletedMilestones;
    
    for (const auto& Milestone : Milestones)
    {
        if (Milestone.Value)
        {
            CompletedMilestones.Add(Milestone.Key);
        }
    }
    
    return CompletedMilestones;
}

void UStudioDirectorSystem::InitializeAgentTasks()
{
    // Initialize all 19 agent tasks with their core responsibilities
    TArray<FString> AgentNames = {
        TEXT("Agent_02_Engine_Architect"),
        TEXT("Agent_03_Core_Systems"),
        TEXT("Agent_04_Performance"),
        TEXT("Agent_05_World_Generator"),
        TEXT("Agent_06_Environment"),
        TEXT("Agent_07_Architecture"),
        TEXT("Agent_08_Lighting"),
        TEXT("Agent_09_Character"),
        TEXT("Agent_10_Animation"),
        TEXT("Agent_11_NPC_Behavior"),
        TEXT("Agent_12_Combat_AI"),
        TEXT("Agent_13_Crowd_Simulation"),
        TEXT("Agent_14_Quest_Designer"),
        TEXT("Agent_15_Narrative"),
        TEXT("Agent_16_Audio"),
        TEXT("Agent_17_VFX"),
        TEXT("Agent_18_QA"),
        TEXT("Agent_19_Integration")
    };

    TArray<FString> TaskDescriptions = {
        TEXT("Define UE5.5 architecture standards and compilation rules"),
        TEXT("Implement physics, collision, ragdoll systems"),
        TEXT("Ensure 60fps PC / 30fps console targets"),
        TEXT("Expand landscape to 10km2 with 5 biomes"),
        TEXT("Populate biomes with vegetation and props"),
        TEXT("Build prehistoric structures and shelters"),
        TEXT("Establish Cretaceous atmosphere and day/night cycle"),
        TEXT("Create human primitive character with MetaHuman"),
        TEXT("Implement Motion Matching and IK systems"),
        TEXT("Define dinosaur behavior trees and routines"),
        TEXT("Implement tactical combat AI for dinosaurs"),
        TEXT("Setup Mass AI for large dinosaur herds"),
        TEXT("Convert narrative into playable missions"),
        TEXT("Write Game Bible and prehistoric lore"),
        TEXT("Create adaptive music and prehistoric soundscape"),
        TEXT("Implement Niagara effects for environment"),
        TEXT("Test all systems and maintain build stability"),
        TEXT("Integrate all agent outputs into cohesive build")
    };

    // Create tasks for each agent
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentNames[i];
        NewTask.TaskDescription = TaskDescriptions[i];
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = 1.0f + (i * 0.1f); // Sequential priority
        CurrentCycle.AgentTasks.Add(NewTask);
    }

    // Set up critical dependencies
    AddTaskDependency(TEXT("Agent_03_Core_Systems"), TEXT("Agent_02_Engine_Architect"));
    AddTaskDependency(TEXT("Agent_05_World_Generator"), TEXT("Agent_02_Engine_Architect"));
    AddTaskDependency(TEXT("Agent_06_Environment"), TEXT("Agent_05_World_Generator"));
    AddTaskDependency(TEXT("Agent_08_Lighting"), TEXT("Agent_06_Environment"));
    AddTaskDependency(TEXT("Agent_10_Animation"), TEXT("Agent_09_Character"));
    AddTaskDependency(TEXT("Agent_11_NPC_Behavior"), TEXT("Agent_09_Character"));
    AddTaskDependency(TEXT("Agent_12_Combat_AI"), TEXT("Agent_11_NPC_Behavior"));
    AddTaskDependency(TEXT("Agent_14_Quest_Designer"), TEXT("Agent_15_Narrative"));
    AddTaskDependency(TEXT("Agent_18_QA"), TEXT("Agent_17_VFX"));
    AddTaskDependency(TEXT("Agent_19_Integration"), TEXT("Agent_18_QA"));

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agent tasks with dependencies"), CurrentCycle.AgentTasks.Num());
}

void UStudioDirectorSystem::ValidateTaskDependencies()
{
    // Check for circular dependencies and invalid references
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        for (const FString& Dependency : Task.Dependencies)
        {
            const FDir_AgentTask* DepTask = FindAgentTask(Dependency);
            if (!DepTask)
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid dependency: %s depends on non-existent agent %s"), 
                       *Task.AgentName, *Dependency);
            }
        }
    }
}

FDir_AgentTask* UStudioDirectorSystem::FindAgentTask(const FString& AgentName)
{
    for (FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            return &Task;
        }
    }
    return nullptr;
}

const FDir_AgentTask* UStudioDirectorSystem::FindAgentTask(const FString& AgentName) const
{
    for (const FDir_AgentTask& Task : CurrentCycle.AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            return &Task;
        }
    }
    return nullptr;
}