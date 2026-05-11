#include "DirectorCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogDirectorCoordinator, Log, All);

UDirectorCoordinator::UDirectorCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Coordinate every second
    
    // Initialize coordination parameters
    MaxActiveAgents = 19;
    CoordinationRadius = 10000.0f;
    TaskPriorityThreshold = 0.5f;
    
    bEnableAgentCoordination = true;
    bEnableTaskDistribution = true;
    bEnableConflictResolution = true;
    
    ActiveAgentCount = 0;
    CompletedTaskCount = 0;
    PendingTaskCount = 0;
    
    LastCoordinationTime = 0.0f;
    CoordinationInterval = 2.0f; // Coordinate every 2 seconds
    
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260511_001");
    StudioDirectorStatus = EDir_AgentStatus::Active;
}

void UDirectorCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Director Coordinator initialized for cycle: %s"), *CurrentCycleID);
    
    // Cache world reference
    CachedWorld = GetWorld();
    if (!CachedWorld)
    {
        UE_LOG(LogDirectorCoordinator, Error, TEXT("Failed to get world reference"));
        return;
    }
    
    // Initialize coordination system
    InitializeCoordinationSystem();
}

void UDirectorCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!CachedWorld)
        return;
    
    float CurrentTime = CachedWorld->GetTimeSeconds();
    
    // Run coordination at specified intervals
    if (CurrentTime - LastCoordinationTime >= CoordinationInterval)
    {
        PerformAgentCoordination();
        LastCoordinationTime = CurrentTime;
    }
}

void UDirectorCoordinator::InitializeCoordinationSystem()
{
    if (!CachedWorld)
        return;
    
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Initializing agent coordination system"));
    
    // Initialize agent registry
    InitializeAgentRegistry();
    
    // Set up task management
    InitializeTaskManagement();
    
    // Initialize conflict resolution
    InitializeConflictResolution();
    
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Director coordination system initialized"));
}

void UDirectorCoordinator::InitializeAgentRegistry()
{
    AgentRegistry.Empty();
    ActiveAgentCount = 0;
    
    // Register all 19 production agents
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
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentInfo AgentInfo;
        AgentInfo.AgentID = i + 1;
        AgentInfo.AgentName = AgentNames[i];
        AgentInfo.Status = EDir_AgentStatus::Idle;
        AgentInfo.Priority = 1.0f;
        AgentInfo.LastActiveTime = 0.0f;
        AgentInfo.TasksCompleted = 0;
        AgentInfo.TasksPending = 0;
        
        // Set specific priorities based on agent role
        if (i < 3) // Core technical agents
        {
            AgentInfo.Priority = 1.0f;
        }
        else if (i < 8) // World building agents
        {
            AgentInfo.Priority = 0.8f;
        }
        else if (i < 14) // Gameplay agents
        {
            AgentInfo.Priority = 0.6f;
        }
        else // Polish agents
        {
            AgentInfo.Priority = 0.4f;
        }
        
        AgentRegistry.Add(AgentInfo);
    }
    
    ActiveAgentCount = AgentRegistry.Num();
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Registered %d agents in coordination system"), ActiveAgentCount);
}

void UDirectorCoordinator::InitializeTaskManagement()
{
    TaskQueue.Empty();
    CompletedTasks.Empty();
    
    CompletedTaskCount = 0;
    PendingTaskCount = 0;
    
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Task management system initialized"));
}

void UDirectorCoordinator::InitializeConflictResolution()
{
    ConflictQueue.Empty();
    ResolvedConflicts.Empty();
    
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Conflict resolution system initialized"));
}

void UDirectorCoordinator::PerformAgentCoordination()
{
    if (!CachedWorld)
        return;
    
    float CurrentTime = CachedWorld->GetTimeSeconds();
    
    // Update agent statuses
    UpdateAgentStatuses(CurrentTime);
    
    // Process task queue
    ProcessTaskQueue();
    
    // Handle conflicts
    ResolveConflicts();
    
    // Update coordination metrics
    UpdateCoordinationMetrics();
    
    UE_LOG(LogDirectorCoordinator, VeryVerbose, TEXT("Agent coordination cycle complete"));
}

void UDirectorCoordinator::UpdateAgentStatuses(float CurrentTime)
{
    for (FDir_AgentInfo& Agent : AgentRegistry)
    {
        // Update agent activity based on time
        if (CurrentTime - Agent.LastActiveTime > 300.0f) // 5 minutes inactive
        {
            if (Agent.Status == EDir_AgentStatus::Active)
            {
                Agent.Status = EDir_AgentStatus::Idle;
                UE_LOG(LogDirectorCoordinator, Log, TEXT("Agent %s marked as idle"), *Agent.AgentName);
            }
        }
        
        // Check for blocked agents
        if (Agent.TasksPending > 5 && Agent.Status == EDir_AgentStatus::Active)
        {
            Agent.Status = EDir_AgentStatus::Blocked;
            UE_LOG(LogDirectorCoordinator, Warning, TEXT("Agent %s marked as blocked (too many pending tasks)"), *Agent.AgentName);
        }
    }
}

void UDirectorCoordinator::ProcessTaskQueue()
{
    if (TaskQueue.Num() == 0)
        return;
    
    // Sort tasks by priority
    TaskQueue.Sort([](const FDir_TaskInfo& A, const FDir_TaskInfo& B) {
        return A.Priority > B.Priority;
    });
    
    // Process high-priority tasks
    for (int32 i = TaskQueue.Num() - 1; i >= 0; i--)
    {
        FDir_TaskInfo& Task = TaskQueue[i];
        
        if (Task.Priority >= TaskPriorityThreshold)
        {
            if (AssignTaskToAgent(Task))
            {
                // Move to completed tasks
                CompletedTasks.Add(Task);
                TaskQueue.RemoveAt(i);
                CompletedTaskCount++;
                PendingTaskCount--;
            }
        }
    }
}

bool UDirectorCoordinator::AssignTaskToAgent(FDir_TaskInfo& Task)
{
    // Find best available agent for the task
    FDir_AgentInfo* BestAgent = nullptr;
    float BestScore = 0.0f;
    
    for (FDir_AgentInfo& Agent : AgentRegistry)
    {
        if (Agent.Status == EDir_AgentStatus::Idle || Agent.Status == EDir_AgentStatus::Active)
        {
            float Score = CalculateAgentTaskScore(Agent, Task);
            if (Score > BestScore)
            {
                BestScore = Score;
                BestAgent = &Agent;
            }
        }
    }
    
    if (BestAgent)
    {
        // Assign task to agent
        Task.AssignedAgentID = BestAgent->AgentID;
        Task.Status = EDir_TaskStatus::InProgress;
        
        BestAgent->Status = EDir_AgentStatus::Active;
        BestAgent->TasksPending++;
        BestAgent->LastActiveTime = CachedWorld->GetTimeSeconds();
        
        UE_LOG(LogDirectorCoordinator, Log, TEXT("Task '%s' assigned to agent %s"), 
               *Task.TaskName, *BestAgent->AgentName);
        
        return true;
    }
    
    return false;
}

float UDirectorCoordinator::CalculateAgentTaskScore(const FDir_AgentInfo& Agent, const FDir_TaskInfo& Task)
{
    float Score = Agent.Priority;
    
    // Prefer agents with fewer pending tasks
    Score -= Agent.TasksPending * 0.1f;
    
    // Prefer agents that match task type
    if (Task.TaskType == EDir_TaskType::Architecture && Agent.AgentID == 2) // Engine Architect
        Score += 0.5f;
    else if (Task.TaskType == EDir_TaskType::Programming && Agent.AgentID == 3) // Core Systems
        Score += 0.5f;
    else if (Task.TaskType == EDir_TaskType::WorldGeneration && Agent.AgentID == 5) // World Generator
        Score += 0.5f;
    else if (Task.TaskType == EDir_TaskType::Art && (Agent.AgentID == 6 || Agent.AgentID == 9)) // Artists
        Score += 0.5f;
    
    return FMath::Max(0.0f, Score);
}

void UDirectorCoordinator::ResolveConflicts()
{
    if (ConflictQueue.Num() == 0)
        return;
    
    for (int32 i = ConflictQueue.Num() - 1; i >= 0; i--)
    {
        FDir_ConflictInfo& Conflict = ConflictQueue[i];
        
        if (ResolveConflict(Conflict))
        {
            ResolvedConflicts.Add(Conflict);
            ConflictQueue.RemoveAt(i);
        }
    }
}

bool UDirectorCoordinator::ResolveConflict(FDir_ConflictInfo& Conflict)
{
    // Simple conflict resolution based on agent priority
    FDir_AgentInfo* Agent1 = GetAgentByID(Conflict.AgentID1);
    FDir_AgentInfo* Agent2 = GetAgentByID(Conflict.AgentID2);
    
    if (!Agent1 || !Agent2)
        return false;
    
    // Higher priority agent wins
    if (Agent1->Priority > Agent2->Priority)
    {
        Conflict.Resolution = FString::Printf(TEXT("Agent %s takes priority"), *Agent1->AgentName);
        Conflict.WinnerAgentID = Agent1->AgentID;
    }
    else
    {
        Conflict.Resolution = FString::Printf(TEXT("Agent %s takes priority"), *Agent2->AgentName);
        Conflict.WinnerAgentID = Agent2->AgentID;
    }
    
    Conflict.Status = EDir_ConflictStatus::Resolved;
    
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Conflict resolved: %s"), *Conflict.Resolution);
    
    return true;
}

FDir_AgentInfo* UDirectorCoordinator::GetAgentByID(int32 AgentID)
{
    for (FDir_AgentInfo& Agent : AgentRegistry)
    {
        if (Agent.AgentID == AgentID)
        {
            return &Agent;
        }
    }
    return nullptr;
}

void UDirectorCoordinator::UpdateCoordinationMetrics()
{
    // Update pending task count
    PendingTaskCount = TaskQueue.Num();
    
    // Count active agents
    int32 ActiveCount = 0;
    for (const FDir_AgentInfo& Agent : AgentRegistry)
    {
        if (Agent.Status == EDir_AgentStatus::Active)
        {
            ActiveCount++;
        }
    }
    ActiveAgentCount = ActiveCount;
}

void UDirectorCoordinator::AddTask(const FString& TaskName, EDir_TaskType TaskType, float Priority, int32 RequesterAgentID)
{
    FDir_TaskInfo NewTask;
    NewTask.TaskID = TaskQueue.Num() + CompletedTasks.Num() + 1;
    NewTask.TaskName = TaskName;
    NewTask.TaskType = TaskType;
    NewTask.Priority = Priority;
    NewTask.RequesterAgentID = RequesterAgentID;
    NewTask.AssignedAgentID = -1;
    NewTask.Status = EDir_TaskStatus::Pending;
    NewTask.CreationTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    
    TaskQueue.Add(NewTask);
    PendingTaskCount++;
    
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Task added: %s (Priority: %.2f)"), *TaskName, Priority);
}

void UDirectorCoordinator::ReportConflict(int32 Agent1ID, int32 Agent2ID, const FString& ConflictDescription)
{
    FDir_ConflictInfo NewConflict;
    NewConflict.ConflictID = ConflictQueue.Num() + ResolvedConflicts.Num() + 1;
    NewConflict.AgentID1 = Agent1ID;
    NewConflict.AgentID2 = Agent2ID;
    NewConflict.Description = ConflictDescription;
    NewConflict.Status = EDir_ConflictStatus::Pending;
    NewConflict.WinnerAgentID = -1;
    NewConflict.CreationTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
    
    ConflictQueue.Add(NewConflict);
    
    UE_LOG(LogDirectorCoordinator, Warning, TEXT("Conflict reported between agents %d and %d: %s"), 
           Agent1ID, Agent2ID, *ConflictDescription);
}

void UDirectorCoordinator::GetCoordinationStats(FDir_CoordinationStats& OutStats)
{
    OutStats.ActiveAgents = ActiveAgentCount;
    OutStats.TotalAgents = AgentRegistry.Num();
    OutStats.PendingTasks = PendingTaskCount;
    OutStats.CompletedTasks = CompletedTaskCount;
    OutStats.PendingConflicts = ConflictQueue.Num();
    OutStats.ResolvedConflicts = ResolvedConflicts.Num();
    OutStats.CoordinationRadius = CoordinationRadius;
    OutStats.bCoordinationEnabled = bEnableAgentCoordination;
    OutStats.CurrentCycleID = CurrentCycleID;
}

void UDirectorCoordinator::SetCoordinationParameters(float InCoordinationRadius, float InTaskPriorityThreshold)
{
    CoordinationRadius = InCoordinationRadius;
    TaskPriorityThreshold = InTaskPriorityThreshold;
    
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Coordination parameters updated: Radius=%.1f, Priority=%.2f"), 
           CoordinationRadius, TaskPriorityThreshold);
}

void UDirectorCoordinator::EnableCoordination(bool bEnable)
{
    bEnableAgentCoordination = bEnable;
    bEnableTaskDistribution = bEnable;
    bEnableConflictResolution = bEnable;
    
    UE_LOG(LogDirectorCoordinator, Log, TEXT("Director coordination %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}