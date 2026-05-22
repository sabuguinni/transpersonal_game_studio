#include "StudioDirectorCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AStudioDirectorCoordinator::AStudioDirectorCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize production state
    CurrentPhase = EDir_ProductionPhase::Gameplay;
    LastUpdateTime = 0.0f;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260522_005");
    
    // Initialize metrics for Milestone 1
    Metrics.CurrentMilestone = TEXT("Milestone 1 - Walk Around");
    Metrics.FilesCreated = 0;
    Metrics.UE5CommandsExecuted = 0;
    Metrics.ActorsSpawned = 0;
    Metrics.CompletionPercentage = 25.0f; // Based on current progress
}

void AStudioDirectorCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Coordinator initialized - Cycle: %s"), *CurrentCycleID);
}

void AStudioDirectorCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update every 5 seconds
    if (LastUpdateTime >= 5.0f)
    {
        CheckTaskDeadlines();
        UpdateProductionMetrics();
        LogProductionStatus();
        LastUpdateTime = 0.0f;
    }
}

void AStudioDirectorCoordinator::InitializeProductionPipeline()
{
    // Clear existing tasks
    ActiveTasks.Empty();
    
    // Priority Task 1: Player Controller (Agent #9/#10)
    FDir_AgentTask PlayerTask;
    PlayerTask.AgentNumber = 9;
    PlayerTask.TaskDescription = TEXT("Fix TranspersonalCharacter WASD movement, third-person camera with SpringArm, terrain collision");
    PlayerTask.Status = EDir_AgentStatus::Working;
    PlayerTask.Priority = 10.0f;
    PlayerTask.CycleID = CurrentCycleID;
    ActiveTasks.Add(PlayerTask);
    
    // Priority Task 2: Survival Component (Agent #3)
    FDir_AgentTask SurvivalTask;
    SurvivalTask.AgentNumber = 3;
    SurvivalTask.TaskDescription = TEXT("Create SurvivalComponent.h + .cpp with Hunger/Thirst/Health that decrease over time");
    SurvivalTask.Status = EDir_AgentStatus::Working;
    SurvivalTask.Priority = 9.0f;
    SurvivalTask.CycleID = CurrentCycleID;
    ActiveTasks.Add(SurvivalTask);
    
    // Priority Task 3: T-Rex AI (Agent #12)
    FDir_AgentTask TRexTask;
    TRexTask.AgentNumber = 12;
    TRexTask.TaskDescription = TEXT("Implement DinosaurCombatAIController.cpp with BehaviorTree: idle -> detect player radius 5000 -> pursue");
    TRexTask.Status = EDir_AgentStatus::Working;
    TRexTask.Priority = 8.0f;
    TRexTask.CycleID = CurrentCycleID;
    ActiveTasks.Add(TRexTask);
    
    // Priority Task 4: Cleanup (Agent #18)
    FDir_AgentTask CleanupTask;
    CleanupTask.AgentNumber = 18;
    CleanupTask.TaskDescription = TEXT("Delete all .h.disabled files and orphaned headers without corresponding .cpp files");
    CleanupTask.Status = EDir_AgentStatus::Idle;
    CleanupTask.Priority = 7.0f;
    CleanupTask.CycleID = CurrentCycleID;
    ActiveTasks.Add(CleanupTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline initialized with %d priority tasks"), ActiveTasks.Num());
}

void AStudioDirectorCoordinator::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Idle;
    NewTask.Priority = Priority;
    NewTask.CycleID = CurrentCycleID;
    NewTask.StartTime = FDateTime::Now();
    NewTask.Deadline = FDateTime::Now() + FTimespan::FromHours(2);
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%d: %s (Priority: %.1f)"), 
           AgentNumber, *TaskDescription, Priority);
}

void AStudioDirectorCoordinator::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.Status = NewStatus;
            
            if (NewStatus == EDir_AgentStatus::Completed)
            {
                Metrics.FilesCreated += 2; // Assume header + cpp
                Metrics.UE5CommandsExecuted += 1;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d status updated to: %s"), 
                   AgentNumber, *UEnum::GetValueAsString(NewStatus));
            break;
        }
    }
}

bool AStudioDirectorCoordinator::IsAgentAvailable(int32 AgentNumber)
{
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber && 
            (Task.Status == EDir_AgentStatus::Working || Task.Status == EDir_AgentStatus::Blocked))
        {
            return false;
        }
    }
    return true;
}

void AStudioDirectorCoordinator::AdvanceToNextPhase()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::CoreSystems:
            CurrentPhase = EDir_ProductionPhase::WorldBuilding;
            break;
        case EDir_ProductionPhase::WorldBuilding:
            CurrentPhase = EDir_ProductionPhase::Gameplay;
            break;
        case EDir_ProductionPhase::Gameplay:
            CurrentPhase = EDir_ProductionPhase::Polish;
            break;
        case EDir_ProductionPhase::Polish:
            CurrentPhase = EDir_ProductionPhase::Testing;
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production phase advanced to: %s"), 
           *UEnum::GetValueAsString(CurrentPhase));
}

void AStudioDirectorCoordinator::ExecutePriorityTask_PlayerController()
{
    UE_LOG(LogTemp, Warning, TEXT("PRIORITY TASK: Player Controller - Dispatching to Agent #9"));
    AssignTaskToAgent(9, TEXT("URGENT: Fix TranspersonalCharacter movement and camera"), 10.0f);
}

void AStudioDirectorCoordinator::ExecutePriorityTask_SurvivalComponent()
{
    UE_LOG(LogTemp, Warning, TEXT("PRIORITY TASK: Survival Component - Dispatching to Agent #3"));
    AssignTaskToAgent(3, TEXT("URGENT: Create SurvivalComponent with stats"), 9.0f);
}

void AStudioDirectorCoordinator::ExecutePriorityTask_TRexAI()
{
    UE_LOG(LogTemp, Warning, TEXT("PRIORITY TASK: T-Rex AI - Dispatching to Agent #12"));
    AssignTaskToAgent(12, TEXT("URGENT: Implement DinosaurCombatAIController.cpp"), 8.0f);
}

void AStudioDirectorCoordinator::ExecutePriorityTask_CleanupOrphans()
{
    UE_LOG(LogTemp, Warning, TEXT("PRIORITY TASK: Cleanup - Dispatching to Agent #18"));
    AssignTaskToAgent(18, TEXT("URGENT: Remove .h.disabled and orphaned headers"), 7.0f);
}

void AStudioDirectorCoordinator::UpdateProductionMetrics()
{
    int32 CompletedTasks = 0;
    int32 TotalTasks = ActiveTasks.Num();
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    if (TotalTasks > 0)
    {
        Metrics.CompletionPercentage = (float)CompletedTasks / (float)TotalTasks * 100.0f;
    }
    
    // Update actor count from world
    if (UWorld* World = GetWorld())
    {
        Metrics.ActorsSpawned = World->GetCurrentLevel()->Actors.Num();
    }
}

FString AStudioDirectorCoordinator::GetProductionStatusReport()
{
    FString Report = FString::Printf(TEXT("=== STUDIO DIRECTOR STATUS REPORT ===\n"));
    Report += FString::Printf(TEXT("Cycle: %s\n"), *CurrentCycleID);
    Report += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(CurrentPhase));
    Report += FString::Printf(TEXT("Milestone: %s\n"), *Metrics.CurrentMilestone);
    Report += FString::Printf(TEXT("Completion: %.1f%%\n"), Metrics.CompletionPercentage);
    Report += FString::Printf(TEXT("Files Created: %d\n"), Metrics.FilesCreated);
    Report += FString::Printf(TEXT("UE5 Commands: %d\n"), Metrics.UE5CommandsExecuted);
    Report += FString::Printf(TEXT("Actors Spawned: %d\n"), Metrics.ActorsSpawned);
    Report += FString::Printf(TEXT("Active Tasks: %d\n"), ActiveTasks.Num());
    
    return Report;
}

TArray<FDir_AgentTask> AStudioDirectorCoordinator::GetTasksByStatus(EDir_AgentStatus Status)
{
    TArray<FDir_AgentTask> FilteredTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == Status)
        {
            FilteredTasks.Add(Task);
        }
    }
    
    return FilteredTasks;
}

void AStudioDirectorCoordinator::CheckTaskDeadlines()
{
    FDateTime Now = FDateTime::Now();
    
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working && Now > Task.Deadline)
        {
            Task.Status = EDir_AgentStatus::Error;
            UE_LOG(LogTemp, Error, TEXT("Task deadline exceeded for Agent #%d: %s"), 
                   Task.AgentNumber, *Task.TaskDescription);
        }
    }
}

void AStudioDirectorCoordinator::LogProductionStatus()
{
    FString StatusReport = GetProductionStatusReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusReport);
}