#include "StudioDirectorCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

UStudioDirectorCoordinator::UStudioDirectorCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260522_004");
    CurrentActiveAgent = 1;
    bCycleInProgress = false;
}

void UStudioDirectorCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Coordinator initialized for cycle: %s"), *CurrentCycleID);
    
    // Initialize the agent chain
    InitializeAgentChain();
    
    // Update initial production metrics
    UpdateProductionMetrics();
}

void UStudioDirectorCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodically update production metrics
    UpdateProductionMetrics();
    
    // Validate world state
    ValidateWorldState();
    
    // Check agent chain integrity
    CheckAgentChainIntegrity();
}

void UStudioDirectorCoordinator::CreateAgentTask(int32 AgentNumber, const FString& AgentName, const FString& TaskDescription, EDir_TaskPriority Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_TaskStatus::Pending;
    NewTask.CycleID = CurrentCycleID;
    
    // Estimate hours based on priority and task type
    switch (Priority)
    {
        case EDir_TaskPriority::Critical:
            NewTask.EstimatedHours = 2.0f;
            break;
        case EDir_TaskPriority::High:
            NewTask.EstimatedHours = 4.0f;
            break;
        case EDir_TaskPriority::Medium:
            NewTask.EstimatedHours = 6.0f;
            break;
        case EDir_TaskPriority::Low:
            NewTask.EstimatedHours = 8.0f;
            break;
    }
    
    ActiveTasks.Add(NewTask);
    LogTaskUpdate(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Created task for Agent #%d: %s"), AgentNumber, *TaskDescription);
}

void UStudioDirectorCoordinator::UpdateTaskStatus(int32 AgentNumber, EDir_TaskStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber && Task.Status != EDir_TaskStatus::Completed)
        {
            Task.Status = NewStatus;
            LogTaskUpdate(Task);
            
            if (NewStatus == EDir_TaskStatus::Completed)
            {
                UE_LOG(LogTemp, Warning, TEXT("Agent #%d completed task: %s"), AgentNumber, *Task.TaskDescription);
                TriggerNextAgent(AgentNumber);
            }
            break;
        }
    }
}

TArray<FDir_AgentTask> UStudioDirectorCoordinator::GetTasksForAgent(int32 AgentNumber)
{
    TArray<FDir_AgentTask> AgentTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

TArray<FDir_AgentTask> UStudioDirectorCoordinator::GetAllTasks()
{
    return ActiveTasks;
}

FDir_ProductionMetrics UStudioDirectorCoordinator::GetProductionMetrics()
{
    return CurrentMetrics;
}

void UStudioDirectorCoordinator::UpdateProductionMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count all actors in the world
    CurrentMetrics.TotalActorsInWorld = 0;
    CurrentMetrics.DinosaurActorsCount = 0;
    CurrentMetrics.TerrainActorsCount = 0;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor)
        {
            CurrentMetrics.TotalActorsInWorld++;
            
            FString ActorName = Actor->GetName().ToLower();
            
            // Count dinosaurs
            if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("rex")) || 
                ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("triceratops")) ||
                ActorName.Contains(TEXT("brachiosaurus")))
            {
                CurrentMetrics.DinosaurActorsCount++;
            }
            
            // Count terrain
            if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")) ||
                ActorName.Contains(TEXT("ground")))
            {
                CurrentMetrics.TerrainActorsCount++;
            }
        }
    }
    
    // Count completed and pending tasks
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.PendingTasks = 0;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_TaskStatus::Completed)
        {
            CurrentMetrics.CompletedTasks++;
        }
        else
        {
            CurrentMetrics.PendingTasks++;
        }
    }
    
    // Determine build status
    if (CurrentMetrics.DinosaurActorsCount >= 5 && CurrentMetrics.TotalActorsInWorld > 50)
    {
        CurrentMetrics.BuildStatus = TEXT("PLAYABLE");
    }
    else if (CurrentMetrics.TotalActorsInWorld > 10)
    {
        CurrentMetrics.BuildStatus = TEXT("IN_PROGRESS");
    }
    else
    {
        CurrentMetrics.BuildStatus = TEXT("MINIMAL");
    }
}

void UStudioDirectorCoordinator::InitializeAgentChain()
{
    // Create initial tasks for the agent chain
    CreateAgentTask(2, TEXT("Engine Architect"), TEXT("Define core engine architecture and systems"), EDir_TaskPriority::Critical);
    CreateAgentTask(3, TEXT("Core Systems Programmer"), TEXT("Implement physics, collision, and destruction"), EDir_TaskPriority::Critical);
    CreateAgentTask(5, TEXT("Procedural World Generator"), TEXT("Generate terrain and biomes"), EDir_TaskPriority::High);
    CreateAgentTask(6, TEXT("Environment Artist"), TEXT("Populate world with vegetation and props"), EDir_TaskPriority::High);
    CreateAgentTask(9, TEXT("Character Artist"), TEXT("Create playable characters and NPCs"), EDir_TaskPriority::High);
    CreateAgentTask(12, TEXT("Combat & Enemy AI"), TEXT("Implement dinosaur AI and combat"), EDir_TaskPriority::Medium);
    
    bCycleInProgress = true;
    UE_LOG(LogTemp, Warning, TEXT("Agent chain initialized for cycle: %s"), *CurrentCycleID);
}

void UStudioDirectorCoordinator::TriggerNextAgent(int32 CurrentAgentNumber)
{
    // Determine next agent in sequence
    int32 NextAgent = CurrentAgentNumber + 1;
    
    // Cap at agent 19 (Integration & Build Agent)
    if (NextAgent > 19)
    {
        CompleteCycle();
        return;
    }
    
    // Validate dependencies before triggering next agent
    if (ValidateAgentDependencies(NextAgent))
    {
        CurrentActiveAgent = NextAgent;
        UE_LOG(LogTemp, Warning, TEXT("Triggering Agent #%d after completion of Agent #%d"), NextAgent, CurrentAgentNumber);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot trigger Agent #%d - dependencies not met"), NextAgent);
    }
}

bool UStudioDirectorCoordinator::ValidateAgentDependencies(int32 AgentNumber)
{
    // Basic dependency validation
    switch (AgentNumber)
    {
        case 3: // Core Systems needs Engine Architect
            return GetTasksForAgent(2).Num() > 0;
        case 5: // World Generator needs Core Systems
            return GetTasksForAgent(3).Num() > 0;
        case 6: // Environment Artist needs World Generator
            return GetTasksForAgent(5).Num() > 0;
        default:
            return true; // No specific dependencies
    }
}

void UStudioDirectorCoordinator::StartNewCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    CurrentActiveAgent = 1;
    bCycleInProgress = true;
    
    // Clear previous cycle tasks
    ActiveTasks.Empty();
    
    // Initialize new cycle
    InitializeAgentChain();
    
    UE_LOG(LogTemp, Warning, TEXT("Started new production cycle: %s"), *CycleID);
}

void UStudioDirectorCoordinator::CompleteCycle()
{
    bCycleInProgress = false;
    CurrentActiveAgent = 1;
    
    UE_LOG(LogTemp, Warning, TEXT("Completed production cycle: %s"), *CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Final metrics - Actors: %d, Dinosaurs: %d, Status: %s"), 
           CurrentMetrics.TotalActorsInWorld, CurrentMetrics.DinosaurActorsCount, *CurrentMetrics.BuildStatus);
}

void UStudioDirectorCoordinator::ValidateMinPlayableMap()
{
    UpdateProductionMetrics();
    
    bool bIsPlayable = (CurrentMetrics.DinosaurActorsCount >= 5 && 
                       CurrentMetrics.TotalActorsInWorld >= 50);
    
    if (bIsPlayable)
    {
        UE_LOG(LogTemp, Warning, TEXT("MinPlayableMap VALIDATION: PASSED - %d dinosaurs, %d total actors"), 
               CurrentMetrics.DinosaurActorsCount, CurrentMetrics.TotalActorsInWorld);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MinPlayableMap VALIDATION: FAILED - Need 5+ dinosaurs and 50+ actors"));
    }
}

void UStudioDirectorCoordinator::LogTaskUpdate(const FDir_AgentTask& Task)
{
    FString StatusString;
    switch (Task.Status)
    {
        case EDir_TaskStatus::Pending:
            StatusString = TEXT("PENDING");
            break;
        case EDir_TaskStatus::InProgress:
            StatusString = TEXT("IN_PROGRESS");
            break;
        case EDir_TaskStatus::Completed:
            StatusString = TEXT("COMPLETED");
            break;
        case EDir_TaskStatus::Blocked:
            StatusString = TEXT("BLOCKED");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TASK UPDATE - Agent #%d (%s): %s [%s]"), 
           Task.AgentNumber, *Task.AgentName, *Task.TaskDescription, *StatusString);
}

void UStudioDirectorCoordinator::ValidateWorldState()
{
    // Ensure minimum world requirements are met
    if (CurrentMetrics.TotalActorsInWorld < 10)
    {
        UE_LOG(LogTemp, Warning, TEXT("World state warning: Only %d actors in world"), CurrentMetrics.TotalActorsInWorld);
    }
}

void UStudioDirectorCoordinator::CheckAgentChainIntegrity()
{
    // Verify agent chain is progressing
    if (bCycleInProgress && CurrentMetrics.PendingTasks == 0 && CurrentMetrics.CompletedTasks == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Agent chain integrity error: No active tasks"));
    }
}