#include "Director_CriticalPathManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"

UDir_CriticalPathManager::UDir_CriticalPathManager()
{
    CurrentPhase = EDir_CriticalPathPhase::Phase1_CoreSystems;
    TotalProjectHours = 0.0f;
    CompletedProjectHours = 0.0f;
}

void UDir_CriticalPathManager::InitializeCriticalPath()
{
    UE_LOG(LogTemp, Warning, TEXT("Director_CriticalPathManager: Initializing Critical Path"));
    
    CriticalTasks.Empty();
    PhaseMetrics.Empty();
    
    // Initialize all phases
    InitializePhase1Tasks();
    InitializePhase2Tasks();
    InitializePhase3Tasks();
    InitializePhase4Tasks();
    InitializePhase5Tasks();
    InitializePhase6Tasks();
    
    // Calculate initial metrics
    for (int32 i = 0; i < 6; i++)
    {
        EDir_CriticalPathPhase Phase = static_cast<EDir_CriticalPathPhase>(i);
        CalculatePhaseMetrics(Phase);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Critical Path initialized with %d tasks"), CriticalTasks.Num());
}

void UDir_CriticalPathManager::InitializePhase1Tasks()
{
    // Phase 1: Core Systems (Agents #2-#4)
    FDir_CriticalTask Task;
    
    // Agent #2 - Engine Architect
    Task.TaskName = TEXT("Engine Architecture");
    Task.Description = TEXT("Define core engine architecture and systems integration");
    Task.AgentID = 2;
    Task.Priority = EDir_TaskPriority::Critical;
    Task.Status = EDir_TaskStatus::Complete;
    Task.Phase = EDir_CriticalPathPhase::Phase1_CoreSystems;
    Task.EstimatedHours = 8.0f;
    Task.ActualHours = 8.0f;
    AddCriticalTask(Task);
    
    // Agent #3 - Core Systems
    Task.TaskName = TEXT("Physics Integration");
    Task.Description = TEXT("Implement core physics, collision, and destruction systems");
    Task.AgentID = 3;
    Task.Priority = EDir_TaskPriority::Critical;
    Task.Status = EDir_TaskStatus::Complete;
    Task.Phase = EDir_CriticalPathPhase::Phase1_CoreSystems;
    Task.Dependencies = {2};
    Task.EstimatedHours = 12.0f;
    Task.ActualHours = 12.0f;
    AddCriticalTask(Task);
    
    // Agent #4 - Performance Optimizer
    Task.TaskName = TEXT("Performance Framework");
    Task.Description = TEXT("Establish performance monitoring and optimization systems");
    Task.AgentID = 4;
    Task.Priority = EDir_TaskPriority::High;
    Task.Status = EDir_TaskStatus::Complete;
    Task.Phase = EDir_CriticalPathPhase::Phase1_CoreSystems;
    Task.Dependencies = {3};
    Task.EstimatedHours = 6.0f;
    Task.ActualHours = 6.0f;
    AddCriticalTask(Task);
}

void UDir_CriticalPathManager::InitializePhase2Tasks()
{
    // Phase 2: Character & Movement (Agent #9-#10)
    FDir_CriticalTask Task;
    
    // Agent #9 - Character Artist
    Task.TaskName = TEXT("Player Character");
    Task.Description = TEXT("Create playable character with WASD movement and third-person camera");
    Task.AgentID = 9;
    Task.Priority = EDir_TaskPriority::Critical;
    Task.Status = EDir_TaskStatus::InProgress;
    Task.Phase = EDir_CriticalPathPhase::Phase2_Character;
    Task.Dependencies = {2, 3};
    Task.EstimatedHours = 10.0f;
    Task.ActualHours = 5.0f;
    AddCriticalTask(Task);
    
    // Agent #10 - Animation
    Task.TaskName = TEXT("Character Animation");
    Task.Description = TEXT("Implement basic movement animations and IK foot placement");
    Task.AgentID = 10;
    Task.Priority = EDir_TaskPriority::High;
    Task.Status = EDir_TaskStatus::NotStarted;
    Task.Phase = EDir_CriticalPathPhase::Phase2_Character;
    Task.Dependencies = {9};
    Task.EstimatedHours = 8.0f;
    Task.ActualHours = 0.0f;
    AddCriticalTask(Task);
}

void UDir_CriticalPathManager::InitializePhase3Tasks()
{
    // Phase 3: Environment & Terrain (Agents #5-#8)
    FDir_CriticalTask Task;
    
    // Agent #5 - World Generator
    Task.TaskName = TEXT("Terrain Generation");
    Task.Description = TEXT("Generate playable terrain with height variation and biomes");
    Task.AgentID = 5;
    Task.Priority = EDir_TaskPriority::Critical;
    Task.Status = EDir_TaskStatus::InProgress;
    Task.Phase = EDir_CriticalPathPhase::Phase3_Environment;
    Task.Dependencies = {2, 3};
    Task.EstimatedHours = 12.0f;
    Task.ActualHours = 6.0f;
    AddCriticalTask(Task);
    
    // Agent #6 - Environment Artist
    Task.TaskName = TEXT("Environment Population");
    Task.Description = TEXT("Populate world with vegetation, rocks, and environmental props");
    Task.AgentID = 6;
    Task.Priority = EDir_TaskPriority::High;
    Task.Status = EDir_TaskStatus::NotStarted;
    Task.Phase = EDir_CriticalPathPhase::Phase3_Environment;
    Task.Dependencies = {5};
    Task.EstimatedHours = 10.0f;
    Task.ActualHours = 0.0f;
    AddCriticalTask(Task);
    
    // Agent #8 - Lighting & Atmosphere
    Task.TaskName = TEXT("Lighting System");
    Task.Description = TEXT("Implement day/night cycle and atmospheric lighting");
    Task.AgentID = 8;
    Task.Priority = EDir_TaskPriority::Medium;
    Task.Status = EDir_TaskStatus::NotStarted;
    Task.Phase = EDir_CriticalPathPhase::Phase3_Environment;
    Task.Dependencies = {5, 6};
    Task.EstimatedHours = 8.0f;
    Task.ActualHours = 0.0f;
    AddCriticalTask(Task);
}

void UDir_CriticalPathManager::InitializePhase4Tasks()
{
    // Phase 4: AI & Dinosaurs (Agents #11-#13)
    FDir_CriticalTask Task;
    
    // Agent #12 - Combat & Enemy AI
    Task.TaskName = TEXT("Dinosaur AI");
    Task.Description = TEXT("Implement basic dinosaur AI with detection and pursuit behavior");
    Task.AgentID = 12;
    Task.Priority = EDir_TaskPriority::Critical;
    Task.Status = EDir_TaskStatus::NotStarted;
    Task.Phase = EDir_CriticalPathPhase::Phase4_AI;
    Task.Dependencies = {9, 5};
    Task.EstimatedHours = 14.0f;
    Task.ActualHours = 0.0f;
    AddCriticalTask(Task);
    
    // Agent #11 - NPC Behavior
    Task.TaskName = TEXT("Dinosaur Spawning");
    Task.Description = TEXT("Spawn dinosaur actors in appropriate biome locations");
    Task.AgentID = 11;
    Task.Priority = EDir_TaskPriority::High;
    Task.Status = EDir_TaskStatus::NotStarted;
    Task.Phase = EDir_CriticalPathPhase::Phase4_AI;
    Task.Dependencies = {5, 12};
    Task.EstimatedHours = 6.0f;
    Task.ActualHours = 0.0f;
    AddCriticalTask(Task);
}

void UDir_CriticalPathManager::InitializePhase5Tasks()
{
    // Phase 5: Survival Systems (Agent #1 coordination)
    FDir_CriticalTask Task;
    
    Task.TaskName = TEXT("Survival HUD");
    Task.Description = TEXT("Implement survival stats display (health, hunger, thirst, stamina)");
    Task.AgentID = 1;
    Task.Priority = EDir_TaskPriority::Critical;
    Task.Status = EDir_TaskStatus::Complete;
    Task.Phase = EDir_CriticalPathPhase::Phase5_Survival;
    Task.Dependencies = {9};
    Task.EstimatedHours = 4.0f;
    Task.ActualHours = 4.0f;
    AddCriticalTask(Task);
    
    Task.TaskName = TEXT("Survival Mechanics");
    Task.Description = TEXT("Implement hunger, thirst, and stamina depletion over time");
    Task.AgentID = 1;
    Task.Priority = EDir_TaskPriority::High;
    Task.Status = EDir_TaskStatus::InProgress;
    Task.Phase = EDir_CriticalPathPhase::Phase5_Survival;
    Task.Dependencies = {9};
    Task.EstimatedHours = 6.0f;
    Task.ActualHours = 3.0f;
    AddCriticalTask(Task);
}

void UDir_CriticalPathManager::InitializePhase6Tasks()
{
    // Phase 6: Integration & Polish (Agent #19)
    FDir_CriticalTask Task;
    
    Task.TaskName = TEXT("Integration Testing");
    Task.Description = TEXT("Test all systems integration and fix critical bugs");
    Task.AgentID = 19;
    Task.Priority = EDir_TaskPriority::Critical;
    Task.Status = EDir_TaskStatus::NotStarted;
    Task.Phase = EDir_CriticalPathPhase::Phase6_Integration;
    Task.Dependencies = {9, 5, 12, 1};
    Task.EstimatedHours = 8.0f;
    Task.ActualHours = 0.0f;
    AddCriticalTask(Task);
}

void UDir_CriticalPathManager::UpdateTaskStatus(const FString& TaskName, EDir_TaskStatus NewStatus)
{
    for (FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.TaskName == TaskName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Task '%s' status updated to %d"), *TaskName, (int32)NewStatus);
            
            // Recalculate metrics for the task's phase
            CalculatePhaseMetrics(Task.Phase);
            break;
        }
    }
}

void UDir_CriticalPathManager::BlockTask(const FString& TaskName, const FString& BlockingReason)
{
    for (FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.TaskName == TaskName)
        {
            Task.Status = EDir_TaskStatus::Blocked;
            Task.BlockingIssue = BlockingReason;
            UE_LOG(LogTemp, Error, TEXT("Task '%s' blocked: %s"), *TaskName, *BlockingReason);
            
            CalculatePhaseMetrics(Task.Phase);
            break;
        }
    }
}

void UDir_CriticalPathManager::UnblockTask(const FString& TaskName)
{
    for (FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.TaskName == TaskName && Task.Status == EDir_TaskStatus::Blocked)
        {
            Task.Status = EDir_TaskStatus::InProgress;
            Task.BlockingIssue = TEXT("");
            UE_LOG(LogTemp, Warning, TEXT("Task '%s' unblocked"), *TaskName);
            
            CalculatePhaseMetrics(Task.Phase);
            break;
        }
    }
}

bool UDir_CriticalPathManager::CanAdvanceToPhase(EDir_CriticalPathPhase TargetPhase)
{
    if (TargetPhase <= CurrentPhase)
    {
        return true; // Already at or past this phase
    }
    
    // Check if previous phase is complete
    EDir_CriticalPathPhase PreviousPhase = static_cast<EDir_CriticalPathPhase>((int32)TargetPhase - 1);
    FDir_PhaseMetrics PrevMetrics = GetPhaseMetrics(PreviousPhase);
    
    return PrevMetrics.CompletionPercentage >= 80.0f && PrevMetrics.BlockedTasks == 0;
}

void UDir_CriticalPathManager::AdvanceToNextPhase()
{
    EDir_CriticalPathPhase NextPhase = static_cast<EDir_CriticalPathPhase>((int32)CurrentPhase + 1);
    
    if (CanAdvanceToPhase(NextPhase))
    {
        CurrentPhase = NextPhase;
        UE_LOG(LogTemp, Warning, TEXT("Advanced to Phase %d"), (int32)CurrentPhase);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot advance to next phase - prerequisites not met"));
    }
}

void UDir_CriticalPathManager::AddCriticalTask(const FDir_CriticalTask& NewTask)
{
    CriticalTasks.Add(NewTask);
    TotalProjectHours += NewTask.EstimatedHours;
}

TArray<FDir_CriticalTask> UDir_CriticalPathManager::GetTasksForAgent(int32 AgentID)
{
    TArray<FDir_CriticalTask> AgentTasks;
    
    for (const FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.AgentID == AgentID)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

TArray<FDir_CriticalTask> UDir_CriticalPathManager::GetTasksForPhase(EDir_CriticalPathPhase Phase)
{
    TArray<FDir_CriticalTask> PhaseTasks;
    
    for (const FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.Phase == Phase)
        {
            PhaseTasks.Add(Task);
        }
    }
    
    return PhaseTasks;
}

TArray<FDir_CriticalTask> UDir_CriticalPathManager::GetBlockedTasks()
{
    TArray<FDir_CriticalTask> BlockedTasks;
    
    for (const FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.Status == EDir_TaskStatus::Blocked)
        {
            BlockedTasks.Add(Task);
        }
    }
    
    return BlockedTasks;
}

TArray<FDir_CriticalTask> UDir_CriticalPathManager::GetCriticalTasks()
{
    TArray<FDir_CriticalTask> CritTasks;
    
    for (const FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.Priority == EDir_TaskPriority::Critical)
        {
            CritTasks.Add(Task);
        }
    }
    
    return CritTasks;
}

FDir_PhaseMetrics UDir_CriticalPathManager::GetPhaseMetrics(EDir_CriticalPathPhase Phase)
{
    for (const FDir_PhaseMetrics& Metrics : PhaseMetrics)
    {
        if (Metrics.Phase == Phase)
        {
            return Metrics;
        }
    }
    
    // If not found, calculate and return
    CalculatePhaseMetrics(Phase);
    return GetPhaseMetrics(Phase);
}

void UDir_CriticalPathManager::CalculatePhaseMetrics(EDir_CriticalPathPhase Phase)
{
    FDir_PhaseMetrics Metrics;
    Metrics.Phase = Phase;
    
    TArray<FDir_CriticalTask> PhaseTasks = GetTasksForPhase(Phase);
    
    Metrics.TotalTasks = PhaseTasks.Num();
    Metrics.CompletedTasks = 0;
    Metrics.BlockedTasks = 0;
    Metrics.EstimatedRemainingHours = 0.0f;
    
    for (const FDir_CriticalTask& Task : PhaseTasks)
    {
        if (Task.Status == EDir_TaskStatus::Complete)
        {
            Metrics.CompletedTasks++;
            CompletedProjectHours += Task.ActualHours;
        }
        else if (Task.Status == EDir_TaskStatus::Blocked)
        {
            Metrics.BlockedTasks++;
            Metrics.EstimatedRemainingHours += Task.EstimatedHours - Task.ActualHours;
        }
        else
        {
            Metrics.EstimatedRemainingHours += Task.EstimatedHours - Task.ActualHours;
        }
    }
    
    Metrics.CompletionPercentage = Metrics.TotalTasks > 0 ? 
        (float)Metrics.CompletedTasks / (float)Metrics.TotalTasks * 100.0f : 0.0f;
    
    Metrics.bCanStartNextPhase = Metrics.CompletionPercentage >= 80.0f && Metrics.BlockedTasks == 0;
    
    // Update or add metrics
    bool bFound = false;
    for (FDir_PhaseMetrics& ExistingMetrics : PhaseMetrics)
    {
        if (ExistingMetrics.Phase == Phase)
        {
            ExistingMetrics = Metrics;
            bFound = true;
            break;
        }
    }
    
    if (!bFound)
    {
        PhaseMetrics.Add(Metrics);
    }
}

TArray<FDir_PhaseMetrics> UDir_CriticalPathManager::GetAllPhaseMetrics()
{
    // Ensure all phases have calculated metrics
    for (int32 i = 0; i < 6; i++)
    {
        EDir_CriticalPathPhase Phase = static_cast<EDir_CriticalPathPhase>(i);
        CalculatePhaseMetrics(Phase);
    }
    
    return PhaseMetrics;
}

float UDir_CriticalPathManager::GetOverallCompletionPercentage()
{
    return TotalProjectHours > 0.0f ? (CompletedProjectHours / TotalProjectHours) * 100.0f : 0.0f;
}

float UDir_CriticalPathManager::GetProjectedCompletionHours()
{
    float RemainingHours = 0.0f;
    
    for (const FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.Status != EDir_TaskStatus::Complete)
        {
            RemainingHours += Task.EstimatedHours - Task.ActualHours;
        }
    }
    
    return RemainingHours;
}

TArray<int32> UDir_CriticalPathManager::GetReadyAgents()
{
    TArray<int32> ReadyAgents;
    
    for (const FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.Status == EDir_TaskStatus::NotStarted && CheckDependenciesComplete(Task))
        {
            ReadyAgents.AddUnique(Task.AgentID);
        }
    }
    
    return ReadyAgents;
}

TArray<int32> UDir_CriticalPathManager::GetBlockedAgents()
{
    TArray<int32> BlockedAgents;
    
    for (const FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.Status == EDir_TaskStatus::Blocked)
        {
            BlockedAgents.AddUnique(Task.AgentID);
        }
    }
    
    return BlockedAgents;
}

bool UDir_CriticalPathManager::CheckDependenciesComplete(const FDir_CriticalTask& Task)
{
    for (int32 DepAgentID : Task.Dependencies)
    {
        bool bDependencyMet = false;
        
        for (const FDir_CriticalTask& DepTask : CriticalTasks)
        {
            if (DepTask.AgentID == DepAgentID && DepTask.Status == EDir_TaskStatus::Complete)
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

void UDir_CriticalPathManager::AssignTaskToAgent(const FString& TaskName, int32 AgentID)
{
    for (FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.TaskName == TaskName)
        {
            Task.AgentID = AgentID;
            UE_LOG(LogTemp, Warning, TEXT("Task '%s' assigned to Agent %d"), *TaskName, AgentID);
            break;
        }
    }
}

void UDir_CriticalPathManager::ReassignBlockedTasks()
{
    TArray<FDir_CriticalTask> BlockedTasks = GetBlockedTasks();
    
    for (FDir_CriticalTask& Task : BlockedTasks)
    {
        // Simple reassignment logic - could be more sophisticated
        if (Task.AgentID < 19)
        {
            Task.AgentID++;
            Task.Status = EDir_TaskStatus::NotStarted;
            Task.BlockingIssue = TEXT("");
            UE_LOG(LogTemp, Warning, TEXT("Reassigned blocked task '%s' to Agent %d"), *Task.TaskName, Task.AgentID);
        }
    }
}

bool UDir_CriticalPathManager::ValidateCriticalPath()
{
    TArray<FString> Violations = GetCriticalPathViolations();
    return Violations.Num() == 0;
}

TArray<FString> UDir_CriticalPathManager::GetCriticalPathViolations()
{
    TArray<FString> Violations;
    
    // Check for circular dependencies
    // Check for orphaned tasks
    // Check for missing critical tasks
    
    TArray<FDir_CriticalTask> CriticalTasks = GetCriticalTasks();
    for (const FDir_CriticalTask& Task : CriticalTasks)
    {
        if (Task.Status == EDir_TaskStatus::Blocked)
        {
            Violations.Add(FString::Printf(TEXT("Critical task '%s' is blocked: %s"), *Task.TaskName, *Task.BlockingIssue));
        }
    }
    
    return Violations;
}

void UDir_CriticalPathManager::GenerateCriticalPathReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CRITICAL PATH REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Overall Completion: %.1f%%"), GetOverallCompletionPercentage());
    UE_LOG(LogTemp, Warning, TEXT("Projected Remaining Hours: %.1f"), GetProjectedCompletionHours());
    
    TArray<FDir_PhaseMetrics> AllMetrics = GetAllPhaseMetrics();
    for (const FDir_PhaseMetrics& Metrics : AllMetrics)
    {
        UE_LOG(LogTemp, Warning, TEXT("Phase %d: %.1f%% complete, %d blocked"), 
            (int32)Metrics.Phase, Metrics.CompletionPercentage, Metrics.BlockedTasks);
    }
    
    TArray<FString> Violations = GetCriticalPathViolations();
    if (Violations.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL PATH VIOLATIONS:"));
        for (const FString& Violation : Violations)
        {
            UE_LOG(LogTemp, Error, TEXT("- %s"), *Violation);
        }
    }
}