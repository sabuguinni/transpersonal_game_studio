#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UProductionCoordinator::UProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    CurrentPhase = EDir_ProductionPhase::Foundation;
    TaskUpdateInterval = 5.0f;
    bAutoResolveBlocks = true;
    LastUpdateTime = 0.0f;
    
    // Initialize production metrics
    ProductionMetrics.ProjectStartDate = FDateTime::Now();
    ProductionMetrics.EstimatedCompletionDate = FDateTime::Now() + FTimespan::FromDays(30);
}

void UProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing Studio Director coordination system"));
    
    // Initialize agent tasks for all 19 agents
    InitializeAgentTasks();
    
    // Start production monitoring
    UpdateProductionMetrics();
}

void UProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= TaskUpdateInterval)
    {
        UpdateProductionMetrics();
        CheckForBlockedTasks();
        
        if (bAutoResolveBlocks)
        {
            AutoResolveSimpleBlocks();
        }
        
        LastUpdateTime = 0.0f;
    }
}

void UProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    if (CurrentPhase != NewPhase)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Phase transition from %d to %d"), 
               (int32)CurrentPhase, (int32)NewPhase);
        
        CurrentPhase = NewPhase;
        
        // Trigger phase-specific logic
        switch (NewPhase)
        {
            case EDir_ProductionPhase::Foundation:
                UE_LOG(LogTemp, Warning, TEXT("Entering Foundation Phase - Core systems priority"));
                break;
            case EDir_ProductionPhase::Core:
                UE_LOG(LogTemp, Warning, TEXT("Entering Core Phase - Gameplay systems priority"));
                break;
            case EDir_ProductionPhase::Polish:
                UE_LOG(LogTemp, Warning, TEXT("Entering Polish Phase - Quality and optimization"));
                break;
            default:
                break;
        }
    }
}

void UProductionCoordinator::AddAgentTask(const FDir_AgentTask& NewTask)
{
    AgentTasks.Add(NewTask);
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Added task for Agent #%d: %s"), 
           NewTask.AgentID, *NewTask.TaskDescription);
    
    UpdateProductionMetrics();
}

void UProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            
            if (NewStatus == EDir_AgentStatus::Complete)
            {
                Task.EstimatedCompletion = FDateTime::Now();
                UE_LOG(LogTemp, Warning, TEXT("Agent #%d completed task: %s"), 
                       AgentID, *Task.TaskDescription);
            }
            else if (NewStatus == EDir_AgentStatus::Blocked)
            {
                UE_LOG(LogTemp, Error, TEXT("Agent #%d is blocked: %s"), 
                       AgentID, *Task.TaskDescription);
            }
            
            break;
        }
    }
    
    UpdateProductionMetrics();
}

bool UProductionCoordinator::IsAgentBlocked(int32 AgentID) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID && Task.Status == EDir_AgentStatus::Blocked)
        {
            return true;
        }
    }
    return false;
}

TArray<int32> UProductionCoordinator::GetBlockedAgents() const
{
    TArray<int32> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.AddUnique(Task.AgentID);
        }
    }
    
    return BlockedAgents;
}

void UProductionCoordinator::ResolveAgentDependency(int32 AgentID, int32 DependencyAgentID)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Dependencies.Remove(DependencyAgentID);
            
            if (Task.Dependencies.Num() == 0 && Task.Status == EDir_AgentStatus::Blocked)
            {
                Task.Status = EDir_AgentStatus::Idle;
                UE_LOG(LogTemp, Warning, TEXT("Agent #%d dependency resolved, now available"), AgentID);
            }
            
            break;
        }
    }
}

FDir_ProductionMetrics UProductionCoordinator::CalculateProductionMetrics() const
{
    FDir_ProductionMetrics Metrics = ProductionMetrics;
    
    Metrics.TotalTasks = AgentTasks.Num();
    Metrics.CompletedTasks = 0;
    Metrics.BlockedTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            Metrics.CompletedTasks++;
        }
        else if (Task.Status == EDir_AgentStatus::Blocked)
        {
            Metrics.BlockedTasks++;
        }
    }
    
    if (Metrics.TotalTasks > 0)
    {
        Metrics.OverallProgress = (float)Metrics.CompletedTasks / (float)Metrics.TotalTasks * 100.0f;
    }
    
    return Metrics;
}

float UProductionCoordinator::GetOverallProgress() const
{
    return CalculateProductionMetrics().OverallProgress;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetTasksByStatus(EDir_AgentStatus Status) const
{
    TArray<FDir_AgentTask> FilteredTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == Status)
        {
            FilteredTasks.Add(Task);
        }
    }
    
    return FilteredTasks;
}

TArray<int32> UProductionCoordinator::GetCriticalPath() const
{
    TArray<int32> CriticalPath;
    
    // Find the longest dependency chain
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Dependencies.Num() == 0) // Root tasks
        {
            TArray<int32> Chain = FindDependencyChain(Task.AgentID);
            if (Chain.Num() > CriticalPath.Num())
            {
                CriticalPath = Chain;
            }
        }
    }
    
    return CriticalPath;
}

void UProductionCoordinator::OptimizeTaskSchedule()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Optimizing task schedule"));
    
    // Sort tasks by priority and dependencies
    AgentTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        if (A.Dependencies.Num() != B.Dependencies.Num())
        {
            return A.Dependencies.Num() < B.Dependencies.Num(); // Fewer dependencies first
        }
        return A.Priority > B.Priority; // Higher priority first
    });
    
    // Update estimated completion times based on dependencies
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (CanTaskStart(Task))
        {
            Task.Status = EDir_AgentStatus::Idle;
        }
        else
        {
            Task.Status = EDir_AgentStatus::Blocked;
        }
    }
}

void UProductionCoordinator::PrintProductionStatus() const
{
    FDir_ProductionMetrics Metrics = CalculateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Total Tasks: %d"), Metrics.TotalTasks);
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d"), Metrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked: %d"), Metrics.BlockedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Progress: %.1f%%"), Metrics.OverallProgress);
    
    TArray<int32> CriticalPath = GetCriticalPath();
    UE_LOG(LogTemp, Warning, TEXT("Critical Path Length: %d"), CriticalPath.Num());
}

void UProductionCoordinator::ValidateProductionState()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Validating production state"));
    
    PrintProductionStatus();
    
    // Check for circular dependencies
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TArray<int32> Visited;
        if (HasCircularDependency(Task.AgentID, Visited))
        {
            UE_LOG(LogTemp, Error, TEXT("Circular dependency detected for Agent #%d"), Task.AgentID);
        }
    }
}

void UProductionCoordinator::UpdateProductionMetrics()
{
    ProductionMetrics = CalculateProductionMetrics();
}

void UProductionCoordinator::CheckForBlockedTasks()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working && !CanTaskStart(Task))
        {
            Task.Status = EDir_AgentStatus::Blocked;
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d became blocked"), Task.AgentID);
        }
        else if (Task.Status == EDir_AgentStatus::Blocked && CanTaskStart(Task))
        {
            Task.Status = EDir_AgentStatus::Idle;
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d unblocked"), Task.AgentID);
        }
    }
}

void UProductionCoordinator::AutoResolveSimpleBlocks()
{
    // Auto-resolve blocks where dependencies are complete
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            bool AllDependenciesComplete = true;
            
            for (int32 DepID : Task.Dependencies)
            {
                bool DepComplete = false;
                for (const FDir_AgentTask& DepTask : AgentTasks)
                {
                    if (DepTask.AgentID == DepID && DepTask.Status == EDir_AgentStatus::Complete)
                    {
                        DepComplete = true;
                        break;
                    }
                }
                
                if (!DepComplete)
                {
                    AllDependenciesComplete = false;
                    break;
                }
            }
            
            if (AllDependenciesComplete)
            {
                Task.Status = EDir_AgentStatus::Idle;
                UE_LOG(LogTemp, Warning, TEXT("Auto-resolved block for Agent #%d"), Task.AgentID);
            }
        }
    }
}

bool UProductionCoordinator::CanTaskStart(const FDir_AgentTask& Task) const
{
    // Check if all dependencies are complete
    for (int32 DepID : Task.Dependencies)
    {
        bool DepComplete = false;
        for (const FDir_AgentTask& DepTask : AgentTasks)
        {
            if (DepTask.AgentID == DepID && DepTask.Status == EDir_AgentStatus::Complete)
            {
                DepComplete = true;
                break;
            }
        }
        
        if (!DepComplete)
        {
            return false;
        }
    }
    
    return true;
}

TArray<int32> UProductionCoordinator::FindDependencyChain(int32 StartAgentID) const
{
    TArray<int32> Chain;
    TArray<int32> Visited;
    
    BuildDependencyChain(StartAgentID, Chain, Visited);
    
    return Chain;
}

void UProductionCoordinator::BuildDependencyChain(int32 AgentID, TArray<int32>& Chain, TArray<int32>& Visited) const
{
    if (Visited.Contains(AgentID))
    {
        return; // Avoid infinite loops
    }
    
    Visited.Add(AgentID);
    Chain.Add(AgentID);
    
    // Find tasks that depend on this agent
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Dependencies.Contains(AgentID))
        {
            BuildDependencyChain(Task.AgentID, Chain, Visited);
        }
    }
}

bool UProductionCoordinator::HasCircularDependency(int32 AgentID, TArray<int32>& Visited) const
{
    if (Visited.Contains(AgentID))
    {
        return true; // Circular dependency found
    }
    
    Visited.Add(AgentID);
    
    // Check dependencies
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            for (int32 DepID : Task.Dependencies)
            {
                if (HasCircularDependency(DepID, Visited))
                {
                    return true;
                }
            }
            break;
        }
    }
    
    Visited.Remove(AgentID);
    return false;
}

void UProductionCoordinator::InitializeAgentTasks()
{
    // Initialize tasks for all 19 agents in the production chain
    
    // Agent #2 - Engine Architect (Foundation)
    FDir_AgentTask EngineTask;
    EngineTask.AgentID = 2;
    EngineTask.AgentName = TEXT("Engine Architect");
    EngineTask.TaskDescription = TEXT("Define technical architecture and UE5 integration patterns");
    EngineTask.Priority = 10.0f;
    EngineTask.Status = EDir_AgentStatus::Working;
    AgentTasks.Add(EngineTask);
    
    // Agent #3 - Core Systems (Depends on #2)
    FDir_AgentTask CoreTask;
    CoreTask.AgentID = 3;
    CoreTask.AgentName = TEXT("Core Systems Programmer");
    CoreTask.TaskDescription = TEXT("Implement physics, collision, and core gameplay systems");
    CoreTask.Priority = 9.0f;
    CoreTask.Dependencies.Add(2);
    CoreTask.Status = EDir_AgentStatus::Blocked;
    AgentTasks.Add(CoreTask);
    
    // Agent #5 - World Generator (Depends on #3)
    FDir_AgentTask WorldTask;
    WorldTask.AgentID = 5;
    WorldTask.AgentName = TEXT("Procedural World Generator");
    WorldTask.TaskDescription = TEXT("Generate terrain, biomes, and world structure");
    WorldTask.Priority = 8.0f;
    WorldTask.Dependencies.Add(3);
    WorldTask.Status = EDir_AgentStatus::Blocked;
    AgentTasks.Add(WorldTask);
    
    // Agent #9 - Character Artist (Depends on #3)
    FDir_AgentTask CharTask;
    CharTask.AgentID = 9;
    CharTask.AgentName = TEXT("Character Artist");
    CharTask.TaskDescription = TEXT("Create player character and dinosaur models");
    CharTask.Priority = 8.0f;
    CharTask.Dependencies.Add(3);
    CharTask.Status = EDir_AgentStatus::Blocked;
    AgentTasks.Add(CharTask);
    
    // Agent #12 - Combat AI (Depends on #9)
    FDir_AgentTask CombatTask;
    CombatTask.AgentID = 12;
    CombatTask.AgentName = TEXT("Combat & Enemy AI");
    CombatTask.TaskDescription = TEXT("Implement dinosaur AI and combat systems");
    CombatTask.Priority = 7.0f;
    CombatTask.Dependencies.Add(9);
    CombatTask.Status = EDir_AgentStatus::Blocked;
    AgentTasks.Add(CombatTask);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d agent tasks"), AgentTasks.Num());
}