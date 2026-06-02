#include "Dir_CycleCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADir_CycleCoordinator::ADir_CycleCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default cycle status
    CurrentCycleStatus.CycleID = TEXT("CYCLE_UNKNOWN");
    CurrentCycleStatus.TotalAgents = 20;
    CurrentCycleStatus.CompletedTasks = 0;
    CurrentCycleStatus.CycleProgress = 0.0f;
    CurrentCycleStatus.bBridgeOperational = true;
}

void ADir_CycleCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    LogCoordinationEvent(TEXT("CycleCoordinator initialized"));
    ValidateBridgeStatus();
}

void ADir_CycleCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateCycleProgress();
    CheckAgentDeadlines();
}

void ADir_CycleCoordinator::InitializeCycle(const FString& CycleID)
{
    CurrentCycleStatus.CycleID = CycleID;
    CurrentCycleStatus.CompletedTasks = 0;
    CurrentCycleStatus.CycleProgress = 0.0f;
    CurrentCycleStatus.ActiveTasks.Empty();
    PendingTasks.Empty();

    LogCoordinationEvent(FString::Printf(TEXT("Cycle %s initialized"), *CycleID));

    // Create default tasks for all 20 agents
    for (int32 i = 1; i <= 20; i++)
    {
        FString AgentName = FString::Printf(TEXT("Agent_%02d"), i);
        FString TaskDesc = TEXT("Execute production cycle tasks");
        FString Deliverable = TEXT("Minimum 2 files + UE5 commands");
        
        AssignTaskToAgent(i, TaskDesc, Deliverable, 1.0f);
    }
}

void ADir_CycleCoordinator::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const FString& ExpectedDeliverable, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.AgentName = FString::Printf(TEXT("Agent_%02d"), AgentNumber);
    NewTask.TaskDescription = TaskDescription;
    NewTask.ExpectedDeliverable = ExpectedDeliverable;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;

    CurrentCycleStatus.ActiveTasks.Add(NewTask);
    PendingTasks.Add(NewTask);

    LogCoordinationEvent(FString::Printf(TEXT("Task assigned to Agent %d: %s"), AgentNumber, *TaskDescription));
}

void ADir_CycleCoordinator::MarkTaskCompleted(int32 AgentNumber)
{
    for (FDir_AgentTask& Task : CurrentCycleStatus.ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber && !Task.bIsCompleted)
        {
            Task.bIsCompleted = true;
            CurrentCycleStatus.CompletedTasks++;
            
            LogCoordinationEvent(FString::Printf(TEXT("Agent %d completed task: %s"), AgentNumber, *Task.TaskDescription));
            break;
        }
    }

    // Remove from pending tasks
    PendingTasks.RemoveAll([AgentNumber](const FDir_AgentTask& Task)
    {
        return Task.AgentNumber == AgentNumber;
    });

    UpdateCycleProgress();
}

float ADir_CycleCoordinator::GetCycleProgress() const
{
    return CurrentCycleStatus.CycleProgress;
}

bool ADir_CycleCoordinator::IsCycleComplete() const
{
    return CurrentCycleStatus.CompletedTasks >= CurrentCycleStatus.TotalAgents;
}

void ADir_CycleCoordinator::ValidateBridgeStatus()
{
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentCycleStatus.bBridgeOperational = true;
        LogCoordinationEvent(TEXT("Bridge validation: OPERATIONAL"));
    }
    else
    {
        CurrentCycleStatus.bBridgeOperational = false;
        LogCoordinationEvent(TEXT("Bridge validation: FAILED"));
    }
}

FString ADir_CycleCoordinator::GenerateProductionReport() const
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT - %s ===\n"), *CurrentCycleStatus.CycleID);
    Report += FString::Printf(TEXT("Progress: %.1f%% (%d/%d agents completed)\n"), 
        CurrentCycleStatus.CycleProgress, CurrentCycleStatus.CompletedTasks, CurrentCycleStatus.TotalAgents);
    Report += FString::Printf(TEXT("Bridge Status: %s\n"), 
        CurrentCycleStatus.bBridgeOperational ? TEXT("OPERATIONAL") : TEXT("FAILED"));
    
    Report += TEXT("\nActive Tasks:\n");
    for (const FDir_AgentTask& Task : CurrentCycleStatus.ActiveTasks)
    {
        FString Status = Task.bIsCompleted ? TEXT("COMPLETED") : TEXT("PENDING");
        Report += FString::Printf(TEXT("  %s: %s [%s]\n"), *Task.AgentName, *Task.TaskDescription, *Status);
    }

    Report += TEXT("\nPending Deliverables:\n");
    for (const FDir_AgentTask& Task : PendingTasks)
    {
        Report += FString::Printf(TEXT("  %s: %s\n"), *Task.AgentName, *Task.ExpectedDeliverable);
    }

    return Report;
}

void ADir_CycleCoordinator::DebugPrintCycleStatus()
{
    FString StatusReport = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusReport);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, StatusReport);
    }
}

void ADir_CycleCoordinator::UpdateCycleProgress()
{
    if (CurrentCycleStatus.TotalAgents > 0)
    {
        CurrentCycleStatus.CycleProgress = (float)CurrentCycleStatus.CompletedTasks / (float)CurrentCycleStatus.TotalAgents * 100.0f;
    }
}

void ADir_CycleCoordinator::CheckAgentDeadlines()
{
    // Check for agents that might be stuck or taking too long
    static float LastDeadlineCheck = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastDeadlineCheck > 30.0f) // Check every 30 seconds
    {
        LastDeadlineCheck = CurrentTime;
        
        if (PendingTasks.Num() > 0)
        {
            LogCoordinationEvent(FString::Printf(TEXT("Deadline check: %d agents still pending"), PendingTasks.Num()));
        }
    }
}

void ADir_CycleCoordinator::LogCoordinationEvent(const FString& Event)
{
    UE_LOG(LogTemp, Log, TEXT("[CycleCoordinator] %s"), *Event);
    
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("[%s] %s"), *CurrentCycleStatus.CycleID, *Event);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, LogMessage);
    }
}