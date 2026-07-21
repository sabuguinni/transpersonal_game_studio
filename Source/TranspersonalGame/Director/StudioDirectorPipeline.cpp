#include "StudioDirectorPipeline.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UStudioDirectorPipeline::UStudioDirectorPipeline()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
}

void UStudioDirectorPipeline::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Pipeline initialized"));
}

void UStudioDirectorPipeline::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bProductionActive && !bEmergencyStop)
    {
        // Update agent task timeouts
        for (FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.Status == EDir_AgentStatus::Working)
            {
                FTimespan ElapsedTime = FDateTime::Now() - Task.StartTime;
                if (ElapsedTime > FTimespan::FromHours(2))
                {
                    UE_LOG(LogTemp, Error, TEXT("Agent %d task timeout: %s"), Task.AgentID, *Task.TaskDescription);
                    Task.Status = EDir_AgentStatus::Failed;
                }
            }
        }
        
        // Check milestone progress
        UpdateCycleMetrics();
    }
}

void UStudioDirectorPipeline::InitializeProductionPipeline()
{
    InitializeAgentRegistry();
    InitializeDefaultMilestones();
    
    bProductionActive = true;
    CycleStartTime = FDateTime::Now();
    CurrentCycleNumber = 1;
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline initialized with %d agents"), AgentNames.Num());
}

void UStudioDirectorPipeline::StartProductionCycle()
{
    if (bEmergencyStop)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start cycle - emergency stop active"));
        return;
    }
    
    CurrentCycleNumber++;
    CycleStartTime = FDateTime::Now();
    
    // Reset agent tasks for new cycle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed || Task.Status == EDir_AgentStatus::Failed)
        {
            Task.Status = EDir_AgentStatus::Idle;
            Task.StartTime = FDateTime::Now();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Starting production cycle %d"), CurrentCycleNumber);
}

bool UStudioDirectorPipeline::ValidateMilestone(EDir_MilestoneType MilestoneType)
{
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.Type == MilestoneType)
        {
            if (Milestone.bIsCompleted)
            {
                return true;
            }
            
            // Check if all required agents have completed their tasks
            int32 CompletedAgents = 0;
            for (int32 AgentID : Milestone.RequiredAgents)
            {
                for (const FDir_AgentTask& Task : AgentTasks)
                {
                    if (Task.AgentID == AgentID && Task.Status == EDir_AgentStatus::Completed)
                    {
                        CompletedAgents++;
                        break;
                    }
                }
            }
            
            float Progress = float(CompletedAgents) / float(Milestone.RequiredAgents.Num());
            
            // Update milestone progress
            for (FDir_ProductionMilestone& MutableMilestone : ProductionMilestones)
            {
                if (MutableMilestone.Type == MilestoneType)
                {
                    MutableMilestone.CompletionPercentage = Progress * 100.0f;
                    if (Progress >= 1.0f)
                    {
                        MutableMilestone.bIsCompleted = true;
                        NotifyMilestoneCompletion(MilestoneType);
                        return true;
                    }
                    break;
                }
            }
            
            return false;
        }
    }
    
    return false;
}

void UStudioDirectorPipeline::BlockProductionPipeline(const FString& Reason)
{
    bEmergencyStop = true;
    bProductionActive = false;
    
    UE_LOG(LogTemp, Error, TEXT("PRODUCTION BLOCKED: %s"), *Reason);
    
    // Notify all agents to stop
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Blocked;
        }
    }
}

void UStudioDirectorPipeline::ResumeProductionPipeline()
{
    bEmergencyStop = false;
    bProductionActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline resumed"));
    
    // Resume blocked agents
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            Task.Status = EDir_AgentStatus::Idle;
        }
    }
}

void UStudioDirectorPipeline::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const TArray<int32>& Dependencies)
{
    // Find existing task or create new one
    FDir_AgentTask* ExistingTask = nullptr;
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            ExistingTask = &Task;
            break;
        }
    }
    
    if (!ExistingTask)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = AgentID;
        NewTask.AgentName = AgentNames.Contains(AgentID) ? AgentNames[AgentID] : FString::Printf(TEXT("Agent_%d"), AgentID);
        AgentTasks.Add(NewTask);
        ExistingTask = &AgentTasks.Last();
    }
    
    ExistingTask->TaskDescription = TaskDescription;
    ExistingTask->Dependencies = Dependencies;
    ExistingTask->Status = EDir_AgentStatus::Idle;
    ExistingTask->StartTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("Assigned task to Agent %d: %s"), AgentID, *TaskDescription);
}

void UStudioDirectorPipeline::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            EDir_AgentStatus OldStatus = Task.Status;
            Task.Status = NewStatus;
            
            UE_LOG(LogTemp, Log, TEXT("Agent %d status: %d -> %d"), AgentID, (int32)OldStatus, (int32)NewStatus);
            
            if (NewStatus == EDir_AgentStatus::Working)
            {
                Task.StartTime = FDateTime::Now();
            }
            break;
        }
    }
}

TArray<FDir_AgentTask> UStudioDirectorPipeline::GetPendingTasks()
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle && CanAgentStart(Task.AgentID))
        {
            PendingTasks.Add(Task);
        }
    }
    
    return PendingTasks;
}

bool UStudioDirectorPipeline::CanAgentStart(int32 AgentID)
{
    if (bEmergencyStop)
    {
        return false;
    }
    
    // Find the agent's task
    const FDir_AgentTask* AgentTask = nullptr;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            AgentTask = &Task;
            break;
        }
    }
    
    if (!AgentTask)
    {
        return true; // No task assigned yet
    }
    
    // Check if all dependencies are completed
    return ValidateAgentDependencies(AgentID);
}

void UStudioDirectorPipeline::CreateMilestone(EDir_MilestoneType Type, const FString& Name, const TArray<int32>& RequiredAgents)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.Type = Type;
    NewMilestone.Name = Name;
    NewMilestone.RequiredAgents = RequiredAgents;
    NewMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(1);
    
    ProductionMilestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("Created milestone: %s"), *Name);
}

void UStudioDirectorPipeline::UpdateMilestoneProgress(EDir_MilestoneType Type, float Progress)
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.Type == Type)
        {
            Milestone.CompletionPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            
            if (Milestone.CompletionPercentage >= 100.0f && !Milestone.bIsCompleted)
            {
                Milestone.bIsCompleted = true;
                NotifyMilestoneCompletion(Type);
            }
            break;
        }
    }
}

FDir_ProductionMilestone UStudioDirectorPipeline::GetCurrentMilestone()
{
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (!Milestone.bIsCompleted)
        {
            return Milestone;
        }
    }
    
    // Return empty milestone if all completed
    return FDir_ProductionMilestone();
}

void UStudioDirectorPipeline::TriggerEmergencyStop(const FString& Reason)
{
    BlockProductionPipeline(FString::Printf(TEXT("EMERGENCY STOP: %s"), *Reason));
    
    // Log critical error
    UE_LOG(LogTemp, Error, TEXT("=== EMERGENCY STOP TRIGGERED ==="));
    UE_LOG(LogTemp, Error, TEXT("Reason: %s"), *Reason);
    UE_LOG(LogTemp, Error, TEXT("Cycle: %d"), CurrentCycleNumber);
    UE_LOG(LogTemp, Error, TEXT("Time: %s"), *FDateTime::Now().ToString());
}

void UStudioDirectorPipeline::InitiateRollback(int32 CyclesBack)
{
    UE_LOG(LogTemp, Warning, TEXT("Initiating rollback %d cycles"), CyclesBack);
    
    // Reset to previous stable state
    CurrentCycleNumber = FMath::Max(1, CurrentCycleNumber - CyclesBack);
    
    // Reset all agents to idle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.StartTime = FDateTime::Now();
    }
    
    ResumeProductionPipeline();
}

void UStudioDirectorPipeline::ForceAgentRestart(int32 AgentID)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = EDir_AgentStatus::Idle;
            Task.StartTime = FDateTime::Now();
            UE_LOG(LogTemp, Warning, TEXT("Force restarted Agent %d"), AgentID);
            break;
        }
    }
}

float UStudioDirectorPipeline::GetOverallProgress()
{
    if (ProductionMilestones.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        TotalProgress += Milestone.CompletionPercentage;
    }
    
    return TotalProgress / float(ProductionMilestones.Num());
}

FString UStudioDirectorPipeline::GetProductionReport()
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT CYCLE %d ===\n"), CurrentCycleNumber);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), GetOverallProgress());
    Report += FString::Printf(TEXT("Active Agents: %d\n"), AgentTasks.Num());
    Report += FString::Printf(TEXT("Emergency Stop: %s\n"), bEmergencyStop ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\nMilestones:\n");
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        Report += FString::Printf(TEXT("- %s: %.1f%% %s\n"), 
            *Milestone.Name, 
            Milestone.CompletionPercentage,
            Milestone.bIsCompleted ? TEXT("[COMPLETED]") : TEXT("[IN PROGRESS]"));
    }
    
    return Report;
}

bool UStudioDirectorPipeline::IsProductionOnSchedule()
{
    FTimespan ElapsedTime = FDateTime::Now() - CycleStartTime;
    float TargetProgress = (ElapsedTime.GetTotalHours() / 24.0f) * 100.0f; // 24h per cycle target
    
    return GetOverallProgress() >= TargetProgress;
}

void UStudioDirectorPipeline::InitializeAgentRegistry()
{
    AgentNames.Empty();
    AgentNames.Add(1, TEXT("Studio Director"));
    AgentNames.Add(2, TEXT("Engine Architect"));
    AgentNames.Add(3, TEXT("Core Systems Programmer"));
    AgentNames.Add(4, TEXT("Performance Optimizer"));
    AgentNames.Add(5, TEXT("Procedural World Generator"));
    AgentNames.Add(6, TEXT("Environment Artist"));
    AgentNames.Add(7, TEXT("Architecture & Interior Agent"));
    AgentNames.Add(8, TEXT("Lighting & Atmosphere Agent"));
    AgentNames.Add(9, TEXT("Character Artist Agent"));
    AgentNames.Add(10, TEXT("Animation Agent"));
    AgentNames.Add(11, TEXT("NPC Behavior Agent"));
    AgentNames.Add(12, TEXT("Combat & Enemy AI Agent"));
    AgentNames.Add(13, TEXT("Crowd & Traffic Simulation"));
    AgentNames.Add(14, TEXT("Quest & Mission Designer"));
    AgentNames.Add(15, TEXT("Narrative & Dialogue Agent"));
    AgentNames.Add(16, TEXT("Audio Agent"));
    AgentNames.Add(17, TEXT("VFX Agent"));
    AgentNames.Add(18, TEXT("QA & Testing Agent"));
    AgentNames.Add(19, TEXT("Integration & Build Agent"));
}

void UStudioDirectorPipeline::InitializeDefaultMilestones()
{
    ProductionMilestones.Empty();
    
    // Milestone 1: Walk Around
    CreateMilestone(EDir_MilestoneType::WalkAround, TEXT("Walk Around Prototype"), {1, 2, 3, 5, 9});
    
    // Milestone 2: Basic Survival
    CreateMilestone(EDir_MilestoneType::BasicSurvival, TEXT("Basic Survival Mechanics"), {3, 4, 12, 16});
    
    // Milestone 3: Dinosaur Encounter
    CreateMilestone(EDir_MilestoneType::DinosaurEncounter, TEXT("Dinosaur AI & Combat"), {11, 12, 13, 17});
    
    // Milestone 4: Crafting System
    CreateMilestone(EDir_MilestoneType::CraftingSystem, TEXT("Primitive Crafting"), {14, 15, 6, 7});
    
    // Milestone 5: Full Prototype
    CreateMilestone(EDir_MilestoneType::FullPrototype, TEXT("Complete Playable Prototype"), {18, 19});
}

void UStudioDirectorPipeline::UpdateCycleMetrics()
{
    FTimespan CycleTime = FDateTime::Now() - CycleStartTime;
    float CycleHours = CycleTime.GetTotalHours();
    
    if (CycleHours > 0.1f) // Update every 6 minutes
    {
        CycleTimes.Add(CycleHours);
        if (CycleTimes.Num() > 10)
        {
            CycleTimes.RemoveAt(0);
        }
        
        float TotalTime = 0.0f;
        for (float Time : CycleTimes)
        {
            TotalTime += Time;
        }
        AverageCycleTime = TotalTime / float(CycleTimes.Num());
    }
}

bool UStudioDirectorPipeline::ValidateAgentDependencies(int32 AgentID)
{
    const FDir_AgentTask* AgentTask = nullptr;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            AgentTask = &Task;
            break;
        }
    }
    
    if (!AgentTask || AgentTask->Dependencies.Num() == 0)
    {
        return true;
    }
    
    // Check if all dependencies are completed
    for (int32 DepAgentID : AgentTask->Dependencies)
    {
        bool bDepCompleted = false;
        for (const FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.AgentID == DepAgentID && Task.Status == EDir_AgentStatus::Completed)
            {
                bDepCompleted = true;
                break;
            }
        }
        
        if (!bDepCompleted)
        {
            return false;
        }
    }
    
    return true;
}

void UStudioDirectorPipeline::NotifyMilestoneCompletion(EDir_MilestoneType CompletedMilestone)
{
    UE_LOG(LogTemp, Warning, TEXT("=== MILESTONE COMPLETED ==="));
    
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.Type == CompletedMilestone)
        {
            UE_LOG(LogTemp, Warning, TEXT("Milestone: %s"), *Milestone.Name);
            break;
        }
    }
    
    // Advance to next milestone
    if (CompletedMilestone == EDir_MilestoneType::WalkAround)
    {
        CurrentMilestone = EDir_MilestoneType::BasicSurvival;
    }
    else if (CompletedMilestone == EDir_MilestoneType::BasicSurvival)
    {
        CurrentMilestone = EDir_MilestoneType::DinosaurEncounter;
    }
    else if (CompletedMilestone == EDir_MilestoneType::DinosaurEncounter)
    {
        CurrentMilestone = EDir_MilestoneType::CraftingSystem;
    }
    else if (CompletedMilestone == EDir_MilestoneType::CraftingSystem)
    {
        CurrentMilestone = EDir_MilestoneType::FullPrototype;
    }
}