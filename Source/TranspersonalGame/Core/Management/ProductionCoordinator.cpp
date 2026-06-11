#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"

UProductionCoordinator::UProductionCoordinator()
{
    ProductionMetrics.CycleNumber = 20; // Starting from current cycle
    ProductionMetrics.CurrentMilestone = TEXT("MILESTONE 1: WALK AROUND");
}

void UProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeAgentTasks();
    SetMilestone(TEXT("MILESTONE 1: WALK AROUND"));
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized for Cycle %d"), ProductionMetrics.CycleNumber);
}

void UProductionCoordinator::Deinitialize()
{
    LogProductionEvent(TEXT("Production Coordinator shutting down"));
    Super::Deinitialize();
}

void UProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    
    FString PhaseNames[] = {
        TEXT("Planning"),
        TEXT("PreProduction"), 
        TEXT("Production"),
        TEXT("Alpha"),
        TEXT("Beta"),
        TEXT("Gold")
    };
    
    FString PhaseName = PhaseNames[static_cast<int32>(NewPhase)];
    LogProductionEvent(FString::Printf(TEXT("Phase changed to: %s"), *PhaseName));
}

void UProductionCoordinator::AssignTask(int32 AgentID, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask* ExistingTask = FindAgentTask(AgentID);
    
    if (ExistingTask)
    {
        ExistingTask->TaskDescription = TaskDescription;
        ExistingTask->Priority = Priority;
        ExistingTask->Status = EDir_AgentStatus::Working;
        ExistingTask->StartTime = FDateTime::Now();
    }
    else
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = AgentID;
        NewTask.TaskDescription = TaskDescription;
        NewTask.Priority = Priority;
        NewTask.Status = EDir_AgentStatus::Working;
        
        // Set agent names
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
        
        if (AgentID >= 1 && AgentID <= 19)
        {
            NewTask.AgentName = AgentNames[AgentID - 1];
        }
        
        AgentTasks.Add(NewTask);
    }
    
    UpdateProductionMetrics();
    LogProductionEvent(FString::Printf(TEXT("Task assigned to Agent #%d: %s"), AgentID, *TaskDescription));
}

void UProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    FDir_AgentTask* Task = FindAgentTask(AgentID);
    if (Task)
    {
        Task->Status = NewStatus;
        UpdateProductionMetrics();
        
        FString StatusNames[] = {
            TEXT("Idle"),
            TEXT("Working"),
            TEXT("Blocked"),
            TEXT("Complete"),
            TEXT("Failed")
        };
        
        FString StatusName = StatusNames[static_cast<int32>(NewStatus)];
        LogProductionEvent(FString::Printf(TEXT("Agent #%d status: %s"), AgentID, *StatusName));
    }
}

void UProductionCoordinator::CompleteTask(int32 AgentID, const FString& Deliverable)
{
    FDir_AgentTask* Task = FindAgentTask(AgentID);
    if (Task)
    {
        Task->Status = EDir_AgentStatus::Complete;
        ProductionMetrics.CompletedDeliverables.Add(Deliverable);
        ProductionMetrics.CompletedTasks++;
        
        UpdateProductionMetrics();
        LogProductionEvent(FString::Printf(TEXT("Agent #%d completed: %s"), AgentID, *Deliverable));
    }
}

void UProductionCoordinator::BlockAgent(int32 AgentID, const FString& BlockingIssue)
{
    FDir_AgentTask* Task = FindAgentTask(AgentID);
    if (Task)
    {
        Task->Status = EDir_AgentStatus::Blocked;
        Task->Dependencies = BlockingIssue;
        ProductionMetrics.CriticalIssues.AddUnique(BlockingIssue);
        
        UpdateProductionMetrics();
        LogProductionEvent(FString::Printf(TEXT("Agent #%d blocked: %s"), AgentID, *BlockingIssue));
    }
}

FDir_ProductionMetrics UProductionCoordinator::GetProductionMetrics() const
{
    return ProductionMetrics;
}

void UProductionCoordinator::IncrementCycle()
{
    ProductionMetrics.CycleNumber++;
    LogProductionEvent(FString::Printf(TEXT("Starting Cycle %d"), ProductionMetrics.CycleNumber));
}

void UProductionCoordinator::SetMilestone(const FString& MilestoneName)
{
    ProductionMetrics.CurrentMilestone = MilestoneName;
    LogProductionEvent(FString::Printf(TEXT("Current milestone: %s"), *MilestoneName));
}

TArray<int32> UProductionCoordinator::GetCriticalPathAgents() const
{
    TArray<int32> CriticalAgents;
    
    // Critical path for Milestone 1: Walk Around
    // Engine Architect -> Core Systems -> World Generator -> Character Artist -> Animation
    CriticalAgents.Add(2); // Engine Architect
    CriticalAgents.Add(3); // Core Systems
    CriticalAgents.Add(5); // World Generator
    CriticalAgents.Add(9); // Character Artist
    CriticalAgents.Add(10); // Animation
    
    return CriticalAgents;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetBlockedTasks() const
{
    TArray<FDir_AgentTask> BlockedTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedTasks.Add(Task);
        }
    }
    
    return BlockedTasks;
}

void UProductionCoordinator::MarkMilestoneComplete(const FString& MilestoneName)
{
    CompletedMilestones.AddUnique(MilestoneName);
    LogProductionEvent(FString::Printf(TEXT("Milestone completed: %s"), *MilestoneName));
}

bool UProductionCoordinator::IsMilestoneComplete(const FString& MilestoneName) const
{
    return CompletedMilestones.Contains(MilestoneName);
}

void UProductionCoordinator::TriggerProductionHalt(const FString& Reason)
{
    bProductionHalted = true;
    HaltReason = Reason;
    
    // Set all working agents to blocked
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Blocked;
            Task.Dependencies = TEXT("Production Halted: ") + Reason;
        }
    }
    
    UpdateProductionMetrics();
    LogProductionEvent(FString::Printf(TEXT("PRODUCTION HALTED: %s"), *Reason));
}

void UProductionCoordinator::ResumeProduction()
{
    bProductionHalted = false;
    HaltReason.Empty();
    
    // Resume blocked agents that were halted
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Dependencies.StartsWith(TEXT("Production Halted:")))
        {
            Task.Status = EDir_AgentStatus::Working;
            Task.Dependencies.Empty();
        }
    }
    
    UpdateProductionMetrics();
    LogProductionEvent(TEXT("Production resumed"));
}

void UProductionCoordinator::InitializeAgentTasks()
{
    // Initialize all 19 agents with their current priorities for Milestone 1
    AssignTask(1, TEXT("Coordinate Milestone 1 completion and agent synchronization"), 10.0f);
    AssignTask(2, TEXT("Finalize core architecture and compilation fixes"), 9.0f);
    AssignTask(3, TEXT("Implement physics and collision systems"), 8.0f);
    AssignTask(4, TEXT("Optimize performance for 60fps target"), 7.0f);
    AssignTask(5, TEXT("Create playable terrain with height variation"), 9.0f);
    AssignTask(6, TEXT("Populate world with vegetation and props"), 6.0f);
    AssignTask(7, TEXT("Add primitive shelters and structures"), 5.0f);
    AssignTask(8, TEXT("Implement day/night cycle and atmospheric lighting"), 6.0f);
    AssignTask(9, TEXT("Create functional player character with movement"), 9.0f);
    AssignTask(10, TEXT("Add character animations and movement blending"), 8.0f);
    AssignTask(11, TEXT("Implement basic NPC behavior systems"), 5.0f);
    AssignTask(12, TEXT("Create dinosaur AI and basic combat"), 7.0f);
    AssignTask(13, TEXT("Set up crowd simulation framework"), 4.0f);
    AssignTask(14, TEXT("Design survival quest framework"), 5.0f);
    AssignTask(15, TEXT("Write game lore and narrative foundation"), 6.0f);
    AssignTask(16, TEXT("Implement environmental audio and music"), 4.0f);
    AssignTask(17, TEXT("Add particle effects for atmosphere"), 5.0f);
    AssignTask(18, TEXT("Test and validate all systems"), 8.0f);
    AssignTask(19, TEXT("Integrate and build playable prototype"), 9.0f);
}

void UProductionCoordinator::UpdateProductionMetrics()
{
    ProductionMetrics.ActiveAgents = 0;
    ProductionMetrics.BlockedAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Working:
                ProductionMetrics.ActiveAgents++;
                break;
            case EDir_AgentStatus::Blocked:
                ProductionMetrics.BlockedAgents++;
                break;
            default:
                break;
        }
    }
    
    // Calculate overall progress
    float TotalTasks = static_cast<float>(AgentTasks.Num());
    if (TotalTasks > 0)
    {
        ProductionMetrics.OverallProgress = (static_cast<float>(ProductionMetrics.CompletedTasks) / TotalTasks) * 100.0f;
    }
}

FDir_AgentTask* UProductionCoordinator::FindAgentTask(int32 AgentID)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            return &Task;
        }
    }
    return nullptr;
}

void UProductionCoordinator::LogProductionEvent(const FString& Event)
{
    FString LogMessage = FString::Printf(TEXT("[CYCLE %d] %s"), ProductionMetrics.CycleNumber, *Event);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, LogMessage);
    }
}