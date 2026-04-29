#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UStudioDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Subsystem Initialized"));
    
    CurrentCycle = 30; // Starting from cycle 030
    
    // Initialize default milestones and agent registry
    InitializeDefaultMilestones();
    InitializeAgentRegistry();
}

void UStudioDirectorSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Subsystem Deinitialized"));
    
    // Clear all data
    ActiveTasks.Empty();
    ProductionMilestones.Empty();
    AgentStatusMap.Empty();
    BlockedAgents.Empty();
    
    Super::Deinitialize();
}

void UStudioDirectorSubsystem::RegisterAgent(const FString& AgentName, EDir_AgentPriority DefaultPriority)
{
    AgentStatusMap.Add(AgentName, EDir_AgentStatus::IDLE);
    
    UE_LOG(LogTemp, Log, TEXT("Registered Agent: %s with Priority: %d"), *AgentName, (int32)DefaultPriority);
}

void UStudioDirectorSubsystem::AssignTask(const FString& AgentName, const FString& TaskDescription, EDir_AgentPriority Priority, const TArray<FString>& Dependencies)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::WAITING;
    NewTask.CycleAssigned = CurrentCycle;
    NewTask.Dependencies = Dependencies;
    
    // Check if dependencies are met
    if (CheckTaskDependencies(NewTask))
    {
        NewTask.Status = EDir_AgentStatus::WORKING;
        UpdateTaskStatus(AgentName, EDir_AgentStatus::WORKING);
    }
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Assigned Task to %s: %s (Priority: %d)"), *AgentName, *TaskDescription, (int32)Priority);
}

void UStudioDirectorSubsystem::CompleteTask(const FString& AgentName, const TArray<FString>& Deliverables)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName && Task.Status == EDir_AgentStatus::WORKING)
        {
            Task.Status = EDir_AgentStatus::COMPLETED;
            Task.CycleCompleted = CurrentCycle;
            Task.Deliverables = Deliverables;
            
            UpdateTaskStatus(AgentName, EDir_AgentStatus::COMPLETED);
            
            UE_LOG(LogTemp, Warning, TEXT("Task Completed by %s with %d deliverables"), *AgentName, Deliverables.Num());
            
            // Check if this completion unlocks other tasks
            for (FDir_AgentTask& WaitingTask : ActiveTasks)
            {
                if (WaitingTask.Status == EDir_AgentStatus::WAITING && CheckTaskDependencies(WaitingTask))
                {
                    WaitingTask.Status = EDir_AgentStatus::WORKING;
                    UpdateTaskStatus(WaitingTask.AgentName, EDir_AgentStatus::WORKING);
                }
            }
            
            // Update milestone progress
            UpdateMilestoneCompletion();
            break;
        }
    }
}

void UStudioDirectorSubsystem::UpdateTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    if (AgentStatusMap.Contains(AgentName))
    {
        AgentStatusMap[AgentName] = NewStatus;
        UE_LOG(LogTemp, Log, TEXT("Agent %s status updated to: %d"), *AgentName, (int32)NewStatus);
    }
}

void UStudioDirectorSubsystem::CreateMilestone(const FString& MilestoneName, const FString& Description, int32 TargetCycle, const TArray<FString>& RequiredDeliverables)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.MilestoneName = MilestoneName;
    NewMilestone.Description = Description;
    NewMilestone.TargetCycle = TargetCycle;
    NewMilestone.RequiredDeliverables = RequiredDeliverables;
    NewMilestone.bIsCompleted = false;
    NewMilestone.CompletionPercentage = 0.0f;
    
    ProductionMilestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("Created Milestone: %s (Target Cycle: %d)"), *MilestoneName, TargetCycle);
}

void UStudioDirectorSubsystem::UpdateMilestoneProgress(const FString& MilestoneName, float ProgressPercentage)
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.CompletionPercentage = FMath::Clamp(ProgressPercentage, 0.0f, 100.0f);
            
            if (Milestone.CompletionPercentage >= 100.0f)
            {
                Milestone.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Milestone COMPLETED: %s"), *MilestoneName);
            }
            break;
        }
    }
}

bool UStudioDirectorSubsystem::IsMilestoneCompleted(const FString& MilestoneName)
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

void UStudioDirectorSubsystem::StartProductionCycle(int32 CycleNumber)
{
    CurrentCycle = CycleNumber;
    
    UE_LOG(LogTemp, Warning, TEXT("=== STARTING PRODUCTION CYCLE %d ==="), CurrentCycle);
    
    // Reset agent statuses for new cycle
    for (auto& AgentPair : AgentStatusMap)
    {
        if (AgentPair.Value == EDir_AgentStatus::COMPLETED)
        {
            AgentPair.Value = EDir_AgentStatus::IDLE;
        }
    }
}

void UStudioDirectorSubsystem::EndProductionCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENDING PRODUCTION CYCLE %d ==="), CurrentCycle);
    
    // Generate cycle report
    FString Report = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("Cycle Report:\n%s"), *Report);
}

TArray<FDir_AgentTask> UStudioDirectorSubsystem::GetPendingTasks()
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::WORKING || Task.Status == EDir_AgentStatus::WAITING)
        {
            PendingTasks.Add(Task);
        }
    }
    
    return PendingTasks;
}

TArray<FDir_AgentTask> UStudioDirectorSubsystem::GetTasksByPriority(EDir_AgentPriority Priority)
{
    TArray<FDir_AgentTask> FilteredTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Priority == Priority)
        {
            FilteredTasks.Add(Task);
        }
    }
    
    return FilteredTasks;
}

bool UStudioDirectorSubsystem::ValidateAgentDeliverables(const FString& AgentName)
{
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName && Task.Status == EDir_AgentStatus::COMPLETED)
        {
            // Basic validation - check if deliverables exist
            return Task.Deliverables.Num() > 0;
        }
    }
    return false;
}

void UStudioDirectorSubsystem::BlockAgent(const FString& AgentName, const FString& Reason)
{
    if (!BlockedAgents.Contains(AgentName))
    {
        BlockedAgents.Add(AgentName);
        UpdateTaskStatus(AgentName, EDir_AgentStatus::BLOCKED);
        
        UE_LOG(LogTemp, Error, TEXT("BLOCKED Agent %s: %s"), *AgentName, *Reason);
    }
}

void UStudioDirectorSubsystem::UnblockAgent(const FString& AgentName)
{
    BlockedAgents.Remove(AgentName);
    UpdateTaskStatus(AgentName, EDir_AgentStatus::IDLE);
    
    UE_LOG(LogTemp, Warning, TEXT("UNBLOCKED Agent %s"), *AgentName);
}

FString UStudioDirectorSubsystem::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT - CYCLE %d ===\n"), CurrentCycle);
    
    int32 CompletedTasks = 0;
    int32 WorkingTasks = 0;
    int32 WaitingTasks = 0;
    int32 FailedTasks = 0;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::COMPLETED: CompletedTasks++; break;
            case EDir_AgentStatus::WORKING: WorkingTasks++; break;
            case EDir_AgentStatus::WAITING: WaitingTasks++; break;
            case EDir_AgentStatus::FAILED: FailedTasks++; break;
        }
    }
    
    Report += FString::Printf(TEXT("Tasks - Completed: %d, Working: %d, Waiting: %d, Failed: %d\n"), 
        CompletedTasks, WorkingTasks, WaitingTasks, FailedTasks);
    
    Report += FString::Printf(TEXT("Blocked Agents: %d\n"), BlockedAgents.Num());
    
    return Report;
}

FString UStudioDirectorSubsystem::GenerateMilestoneReport()
{
    FString Report = TEXT("=== MILESTONE REPORT ===\n");
    
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        FString Status = Milestone.bIsCompleted ? TEXT("COMPLETED") : TEXT("IN PROGRESS");
        Report += FString::Printf(TEXT("%s: %s (%.1f%%)\n"), 
            *Milestone.MilestoneName, *Status, Milestone.CompletionPercentage);
    }
    
    return Report;
}

void UStudioDirectorSubsystem::InitializeDefaultMilestones()
{
    // Milestone 1: Walk Around
    TArray<FString> Milestone1Deliverables;
    Milestone1Deliverables.Add(TEXT("ThirdPersonCharacter with WASD movement"));
    Milestone1Deliverables.Add(TEXT("Camera boom + follow camera"));
    Milestone1Deliverables.Add(TEXT("Landscape with basic terrain"));
    Milestone1Deliverables.Add(TEXT("3-5 static dinosaur meshes"));
    Milestone1Deliverables.Add(TEXT("Directional light + sky atmosphere"));
    
    CreateMilestone(TEXT("Walk Around"), TEXT("Player can walk around world with visible dinosaurs"), 32, Milestone1Deliverables);
    
    // Milestone 2: Basic Survival
    TArray<FString> Milestone2Deliverables;
    Milestone2Deliverables.Add(TEXT("Health/Hunger/Thirst/Stamina UI"));
    Milestone2Deliverables.Add(TEXT("Basic crafting system"));
    Milestone2Deliverables.Add(TEXT("Dinosaur AI behavior"));
    
    CreateMilestone(TEXT("Basic Survival"), TEXT("Core survival mechanics functional"), 40, Milestone2Deliverables);
}

void UStudioDirectorSubsystem::InitializeAgentRegistry()
{
    // Register all 20 agents
    RegisterAgent(TEXT("Agent01_StudioDirector"), EDir_AgentPriority::CRITICAL);
    RegisterAgent(TEXT("Agent02_EngineArchitect"), EDir_AgentPriority::CRITICAL);
    RegisterAgent(TEXT("Agent03_CoreSystems"), EDir_AgentPriority::HIGH);
    RegisterAgent(TEXT("Agent04_Performance"), EDir_AgentPriority::HIGH);
    RegisterAgent(TEXT("Agent05_WorldGenerator"), EDir_AgentPriority::HIGH);
    RegisterAgent(TEXT("Agent06_Environment"), EDir_AgentPriority::MEDIUM);
    RegisterAgent(TEXT("Agent07_Architecture"), EDir_AgentPriority::MEDIUM);
    RegisterAgent(TEXT("Agent08_Lighting"), EDir_AgentPriority::MEDIUM);
    RegisterAgent(TEXT("Agent09_CharacterArtist"), EDir_AgentPriority::HIGH);
    RegisterAgent(TEXT("Agent10_Animation"), EDir_AgentPriority::HIGH);
    RegisterAgent(TEXT("Agent11_NPCBehavior"), EDir_AgentPriority::MEDIUM);
    RegisterAgent(TEXT("Agent12_CombatAI"), EDir_AgentPriority::HIGH);
    RegisterAgent(TEXT("Agent13_CrowdSimulation"), EDir_AgentPriority::LOW);
    RegisterAgent(TEXT("Agent14_QuestDesigner"), EDir_AgentPriority::MEDIUM);
    RegisterAgent(TEXT("Agent15_Narrative"), EDir_AgentPriority::LOW);
    RegisterAgent(TEXT("Agent16_Audio"), EDir_AgentPriority::MEDIUM);
    RegisterAgent(TEXT("Agent17_VFX"), EDir_AgentPriority::MEDIUM);
    RegisterAgent(TEXT("Agent18_QA"), EDir_AgentPriority::CRITICAL);
    RegisterAgent(TEXT("Agent19_Integration"), EDir_AgentPriority::CRITICAL);
    RegisterAgent(TEXT("Agent20_BuildDaemon"), EDir_AgentPriority::CRITICAL);
}

bool UStudioDirectorSubsystem::CheckTaskDependencies(const FDir_AgentTask& Task)
{
    // If no dependencies, task can proceed
    if (Task.Dependencies.Num() == 0)
    {
        return true;
    }
    
    // Check if all dependencies are completed
    for (const FString& Dependency : Task.Dependencies)
    {
        bool DependencyMet = false;
        
        for (const FDir_AgentTask& CompletedTask : ActiveTasks)
        {
            if (CompletedTask.AgentName == Dependency && CompletedTask.Status == EDir_AgentStatus::COMPLETED)
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

void UStudioDirectorSubsystem::UpdateMilestoneCompletion()
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.bIsCompleted)
        {
            continue;
        }
        
        int32 CompletedDeliverables = 0;
        
        // Check how many required deliverables are completed
        for (const FString& Deliverable : Milestone.RequiredDeliverables)
        {
            for (const FDir_AgentTask& Task : ActiveTasks)
            {
                if (Task.Status == EDir_AgentStatus::COMPLETED)
                {
                    for (const FString& TaskDeliverable : Task.Deliverables)
                    {
                        if (TaskDeliverable.Contains(Deliverable))
                        {
                            CompletedDeliverables++;
                            break;
                        }
                    }
                }
            }
        }
        
        float Progress = (float)CompletedDeliverables / (float)Milestone.RequiredDeliverables.Num() * 100.0f;
        UpdateMilestoneProgress(Milestone.MilestoneName, Progress);
    }
}