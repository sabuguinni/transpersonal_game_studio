#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize production settings
    bAutoUpdateMetrics = true;
    MetricsUpdateInterval = 5.0f;
    LastMetricsUpdate = 0.0f;
    
    // Initialize production metrics
    ProductionMetrics = FDir_ProductionMetrics();
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized - Cycle AUTO_007"));
    
    // Setup initial agent tasks
    SetupInitialTasks();
    
    // Update metrics on start
    UpdateProductionMetrics();
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoUpdateMetrics)
    {
        LastMetricsUpdate += DeltaTime;
        if (LastMetricsUpdate >= MetricsUpdateInterval)
        {
            UpdateProductionMetrics();
            LastMetricsUpdate = 0.0f;
        }
    }
    
    // Check task dependencies
    CheckTaskDependencies();
}

void ADir_ProductionCoordinator::AddAgentTask(int32 AgentID, const FString& AgentName, 
    const FString& TaskDescription, EDir_AgentPriority Priority, const FVector& TaskLocation)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_TaskStatus::Pending;
    NewTask.TaskLocation = TaskLocation;
    NewTask.EstimatedDuration = 60.0f; // Default 1 hour
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task added for Agent #%d (%s): %s"), 
           AgentID, *AgentName, *TaskDescription);
    
    UpdateProductionMetrics();
}

void ADir_ProductionCoordinator::UpdateTaskStatus(int32 AgentID, EDir_TaskStatus NewStatus)
{
    FDir_AgentTask* Task = FindTaskByAgentID(AgentID);
    if (Task)
    {
        Task->Status = NewStatus;
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d status updated to: %d"), AgentID, (int32)NewStatus);
        UpdateProductionMetrics();
    }
}

void ADir_ProductionCoordinator::CompleteTask(int32 AgentID)
{
    UpdateTaskStatus(AgentID, EDir_TaskStatus::Completed);
}

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetTasksByPriority(EDir_AgentPriority Priority)
{
    TArray<FDir_AgentTask> FilteredTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Priority == Priority)
        {
            FilteredTasks.Add(Task);
        }
    }
    
    return FilteredTasks;
}

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetTasksByStatus(EDir_TaskStatus Status)
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

void ADir_ProductionCoordinator::UpdateProductionMetrics()
{
    ProductionMetrics.TotalTasks = AgentTasks.Num();
    ProductionMetrics.CompletedTasks = 0;
    ProductionMetrics.PendingTasks = 0;
    ProductionMetrics.BlockedTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_TaskStatus::Completed:
                ProductionMetrics.CompletedTasks++;
                break;
            case EDir_TaskStatus::Pending:
                ProductionMetrics.PendingTasks++;
                break;
            case EDir_TaskStatus::Blocked:
                ProductionMetrics.BlockedTasks++;
                break;
            default:
                break;
        }
    }
    
    if (ProductionMetrics.TotalTasks > 0)
    {
        ProductionMetrics.OverallProgress = 
            (float)ProductionMetrics.CompletedTasks / (float)ProductionMetrics.TotalTasks * 100.0f;
    }
    else
    {
        ProductionMetrics.OverallProgress = 0.0f;
    }
}

FDir_ProductionMetrics ADir_ProductionCoordinator::GetProductionMetrics() const
{
    return ProductionMetrics;
}

float ADir_ProductionCoordinator::GetProgressPercentage() const
{
    return ProductionMetrics.OverallProgress;
}

void ADir_ProductionCoordinator::CreateTaskMarker(const FVector& Location, const FString& MarkerName)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    ATargetPoint* NewMarker = World->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(), Location, FRotator::ZeroRotator);
    if (NewMarker)
    {
        NewMarker->SetActorLabel(MarkerName);
        TaskMarkers.Add(NewMarker);
        
        UE_LOG(LogTemp, Warning, TEXT("Task marker created: %s at %s"), 
               *MarkerName, *Location.ToString());
    }
}

void ADir_ProductionCoordinator::ClearAllTaskMarkers()
{
    for (AActor* Marker : TaskMarkers)
    {
        if (IsValid(Marker))
        {
            Marker->Destroy();
        }
    }
    TaskMarkers.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("All task markers cleared"));
}

void ADir_ProductionCoordinator::InitializeAgentTasks()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Setup critical milestone 1 tasks
    AddAgentTask(5, TEXT("World Generator"), 
                TEXT("Enhance terrain with height variation and biome transitions"), 
                EDir_AgentPriority::Critical, FVector(2000, 0, 200));
    
    AddAgentTask(6, TEXT("Environment Artist"), 
                TEXT("Replace placeholder assets with detailed meshes"), 
                EDir_AgentPriority::Critical, FVector(1500, 1500, 120));
    
    AddAgentTask(9, TEXT("Character Artist"), 
                TEXT("Enhance TranspersonalCharacter with survival UI"), 
                EDir_AgentPriority::Critical, FVector(0, -2000, 150));
    
    AddAgentTask(12, TEXT("Combat AI"), 
                TEXT("Implement basic dinosaur AI and patrol behaviors"), 
                EDir_AgentPriority::Critical, FVector(-2000, 1000, 100));
    
    // Secondary tasks
    AddAgentTask(8, TEXT("Lighting Artist"), 
                TEXT("Improve lighting and atmosphere"), 
                EDir_AgentPriority::High, FVector(0, 0, 300));
    
    AddAgentTask(10, TEXT("Animation Agent"), 
                TEXT("Add character and dinosaur animations"), 
                EDir_AgentPriority::High, FVector(500, -500, 100));
    
    UE_LOG(LogTemp, Warning, TEXT("Agent tasks initialized for Milestone 1"));
}

void ADir_ProductionCoordinator::AssignNextAgentTasks()
{
    // Logic to assign tasks based on dependencies and priorities
    TArray<FDir_AgentTask> CriticalTasks = GetTasksByPriority(EDir_AgentPriority::Critical);
    TArray<FDir_AgentTask> PendingTasks = GetTasksByStatus(EDir_TaskStatus::Pending);
    
    UE_LOG(LogTemp, Warning, TEXT("Critical tasks: %d, Pending tasks: %d"), 
           CriticalTasks.Num(), PendingTasks.Num());
}

bool ADir_ProductionCoordinator::CanAgentProceed(int32 AgentID) const
{
    const FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& T) {
        return T.AgentID == AgentID;
    });
    
    if (!Task) return true; // No task assigned, can proceed
    
    return Task->Status != EDir_TaskStatus::Blocked;
}

void ADir_ProductionCoordinator::DebugPrintAllTasks()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ALL AGENT TASKS ==="));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s): %s [Status: %d, Priority: %d]"), 
               Task.AgentID, *Task.AgentName, *Task.TaskDescription, 
               (int32)Task.Status, (int32)Task.Priority);
    }
}

void ADir_ProductionCoordinator::DebugPrintMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION METRICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Tasks: %d"), ProductionMetrics.TotalTasks);
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d"), ProductionMetrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Pending: %d"), ProductionMetrics.PendingTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked: %d"), ProductionMetrics.BlockedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Progress: %.1f%%"), ProductionMetrics.OverallProgress);
}

void ADir_ProductionCoordinator::SetupInitialTasks()
{
    InitializeAgentTasks();
}

void ADir_ProductionCoordinator::CheckTaskDependencies()
{
    // Check if any blocked tasks can be unblocked
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_TaskStatus::Blocked)
        {
            // Check dependencies (simplified for now)
            bool CanUnblock = true;
            
            if (CanUnblock)
            {
                Task.Status = EDir_TaskStatus::Pending;
                UE_LOG(LogTemp, Warning, TEXT("Task unblocked for Agent #%d"), Task.AgentID);
            }
        }
    }
}

FDir_AgentTask* ADir_ProductionCoordinator::FindTaskByAgentID(int32 AgentID)
{
    return AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& Task) {
        return Task.AgentID == AgentID;
    });
}