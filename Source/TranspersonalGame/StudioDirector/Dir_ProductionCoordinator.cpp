#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    WalkAroundMilestoneStatus = EDir_MilestoneStatus::InProgress;
    CurrentCycleID = 7;
    ProductionBudgetUsed = 53.0f;
    ProductionBudgetTotal = 75.0f;
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized for Cycle %d"), CurrentCycleID);
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Validate agent progress every 5 seconds
    static float ValidationTimer = 0.0f;
    ValidationTimer += DeltaTime;
    
    if (ValidationTimer >= 5.0f)
    {
        ValidateAgentProgress();
        ValidationTimer = 0.0f;
    }
}

void ADir_ProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_MilestoneStatus::NotStarted;
    
    CurrentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription);
}

void ADir_ProductionCoordinator::UpdateTaskStatus(const FString& AgentName, EDir_MilestoneStatus NewStatus)
{
    for (FDir_AgentTask& Task : CurrentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("Task status updated for %s: %d"), *AgentName, (int32)NewStatus);
            break;
        }
    }
}

void ADir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT CYCLE %d ==="), CurrentCycleID);
    UE_LOG(LogTemp, Warning, TEXT("Budget: $%.2f / $%.2f"), ProductionBudgetUsed, ProductionBudgetTotal);
    UE_LOG(LogTemp, Warning, TEXT("Walk Around Milestone: %d"), (int32)WalkAroundMilestoneStatus);
    
    int32 CompletedTasks = 0;
    int32 InProgressTasks = 0;
    int32 BlockedTasks = 0;
    
    for (const FDir_AgentTask& Task : CurrentTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent %s: %s [Status: %d]"), 
               *Task.AgentName, *Task.TaskDescription, (int32)Task.Status);
        
        switch (Task.Status)
        {
            case EDir_MilestoneStatus::Completed:
                CompletedTasks++;
                break;
            case EDir_MilestoneStatus::InProgress:
                InProgressTasks++;
                break;
            case EDir_MilestoneStatus::Blocked:
                BlockedTasks++;
                break;
        }
    }
    
    float OverallProgress = CalculateOverallProgress();
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%% (Completed: %d, In Progress: %d, Blocked: %d)"), 
           OverallProgress, CompletedTasks, InProgressTasks, BlockedTasks);
}

bool ADir_ProductionCoordinator::IsWalkAroundMilestoneComplete()
{
    // Check if all critical tasks are completed
    int32 CriticalTasksCompleted = 0;
    int32 TotalCriticalTasks = 5; // Character, Terrain, Lighting, Dinosaurs, HUD
    
    for (const FDir_AgentTask& Task : CurrentTasks)
    {
        if (Task.Priority >= 9 && Task.Status == EDir_MilestoneStatus::Completed)
        {
            CriticalTasksCompleted++;
        }
    }
    
    bool MilestoneComplete = (CriticalTasksCompleted >= TotalCriticalTasks);
    
    if (MilestoneComplete && WalkAroundMilestoneStatus != EDir_MilestoneStatus::Completed)
    {
        WalkAroundMilestoneStatus = EDir_MilestoneStatus::Completed;
        UE_LOG(LogTemp, Warning, TEXT("MILESTONE 1 'WALK AROUND' COMPLETED!"));
    }
    
    return MilestoneComplete;
}

void ADir_ProductionCoordinator::InitializeProductionTasks()
{
    CurrentTasks.Empty();
    SetupMilestone1Tasks();
}

void ADir_ProductionCoordinator::SetupMilestone1Tasks()
{
    // Critical Path for "Walk Around" Milestone
    AssignTaskToAgent(TEXT("Engine Architect"), TEXT("Define core architecture and compilation rules"), 10);
    AssignTaskToAgent(TEXT("World Generator"), TEXT("Create landscape with hills and terrain variation"), 10);
    AssignTaskToAgent(TEXT("Character Artist"), TEXT("Implement ThirdPersonCharacter with WASD movement"), 10);
    AssignTaskToAgent(TEXT("Animation Agent"), TEXT("Add camera boom and follow camera"), 9);
    AssignTaskToAgent(TEXT("Combat AI"), TEXT("Place 3-5 static dinosaur meshes in world"), 9);
    AssignTaskToAgent(TEXT("Lighting Agent"), TEXT("Configure directional light, sky atmosphere, fog"), 8);
    AssignTaskToAgent(TEXT("Performance Optimizer"), TEXT("Ensure 60fps on PC target"), 7);
    AssignTaskToAgent(TEXT("Audio Agent"), TEXT("Add basic ambient sounds"), 6);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 tasks initialized - %d total tasks"), CurrentTasks.Num());
}

void ADir_ProductionCoordinator::ValidateAgentProgress()
{
    // This would normally check actual game state
    // For now, simulate progress based on cycle progression
    
    float CycleProgress = FMath::Min(CurrentCycleID / 20.0f, 1.0f);
    
    // Update some tasks to show progress
    for (FDir_AgentTask& Task : CurrentTasks)
    {
        if (Task.Status == EDir_MilestoneStatus::NotStarted && FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            Task.Status = EDir_MilestoneStatus::InProgress;
        }
        else if (Task.Status == EDir_MilestoneStatus::InProgress && FMath::RandRange(0.0f, 1.0f) < 0.2f)
        {
            Task.Status = EDir_MilestoneStatus::Completed;
        }
    }
}

float ADir_ProductionCoordinator::CalculateOverallProgress()
{
    if (CurrentTasks.Num() == 0)
        return 0.0f;
    
    int32 CompletedTasks = 0;
    int32 InProgressTasks = 0;
    
    for (const FDir_AgentTask& Task : CurrentTasks)
    {
        if (Task.Status == EDir_MilestoneStatus::Completed)
        {
            CompletedTasks++;
        }
        else if (Task.Status == EDir_MilestoneStatus::InProgress)
        {
            InProgressTasks++;
        }
    }
    
    // Completed tasks = 100%, In Progress = 50%
    float TotalProgress = (CompletedTasks * 1.0f) + (InProgressTasks * 0.5f);
    return (TotalProgress / CurrentTasks.Num()) * 100.0f;
}