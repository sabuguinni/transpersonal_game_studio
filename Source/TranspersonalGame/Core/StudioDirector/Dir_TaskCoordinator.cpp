#include "Dir_TaskCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

ADir_TaskCoordinator::ADir_TaskCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create coordinator mesh component
    CoordinatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoordinatorMesh"));
    CoordinatorMesh->SetupAttachment(RootComponent);

    // Try to load a basic cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        CoordinatorMesh->SetStaticMesh(CubeMesh.Object);
        CoordinatorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }

    // Initialize default values
    CurrentCycle = 5; // Starting at cycle 5
    bDebugMode = true;

    // Set actor properties
    SetActorEnableCollision(false);
    SetActorHiddenInGame(false);
}

void ADir_TaskCoordinator::BeginPlay()
{
    Super::BeginPlay();
    InitializeTaskCoordinator();
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Initialized for Cycle %d"), CurrentCycle);
    }
}

void ADir_TaskCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update cycle metrics periodically
    static float MetricsUpdateTimer = 0.0f;
    MetricsUpdateTimer += DeltaTime;
    
    if (MetricsUpdateTimer >= 30.0f) // Update every 30 seconds
    {
        UpdateCycleMetrics();
        MetricsUpdateTimer = 0.0f;
    }
}

void ADir_TaskCoordinator::InitializeTaskCoordinator()
{
    // Clear existing tasks
    ActiveTasks.Empty();

    // Initialize Cycle 005 critical tasks
    AssignTask(TEXT("Agent #5 - Procedural World Generator"), 
               TEXT("Expand MinPlayableMap terrain to 10km² with 5 distinct biomes"), 
               EDir_AgentPriority::Critical, 
               FVector(5000, 0, 500));

    AssignTask(TEXT("Agent #9 - Character Artist"), 
               TEXT("Create collision-enabled dinosaur actors to replace placeholders"), 
               EDir_AgentPriority::Critical, 
               FVector(-5000, 0, 500));

    AssignTask(TEXT("Agent #12 - Combat & Enemy AI"), 
               TEXT("Implement survival HUD showing player stats"), 
               EDir_AgentPriority::Critical, 
               FVector(0, 5000, 500));

    AssignTask(TEXT("Agent #8 - Lighting & Atmosphere"), 
               TEXT("Establish Cretaceous period atmosphere with tropical lighting"), 
               EDir_AgentPriority::High, 
               FVector(0, -5000, 500));

    AssignTask(TEXT("Agent #10 - Animation"), 
               TEXT("Add basic locomotion to dinosaur actors"), 
               EDir_AgentPriority::High, 
               FVector(2500, 2500, 500));

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Assigned %d tasks for Cycle %d"), 
               ActiveTasks.Num(), CurrentCycle);
    }
}

void ADir_TaskCoordinator::AssignTask(const FString& AgentName, const FString& TaskDescription, 
                                     EDir_AgentPriority Priority, const FVector& MarkerLocation)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_TaskStatus::Pending;
    NewTask.MarkerLocation = MarkerLocation;
    NewTask.CycleAssigned = CurrentCycle;

    ActiveTasks.Add(NewTask);

    // Spawn visual marker for the task
    SpawnTaskMarker(AgentName, MarkerLocation);

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Assigned task to %s - %s"), 
               *AgentName, *TaskDescription);
    }
}

void ADir_TaskCoordinator::UpdateTaskStatus(const FString& AgentName, EDir_TaskStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            
            if (bDebugMode)
            {
                UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Updated %s status to %d"), 
                       *AgentName, (int32)NewStatus);
            }
            break;
        }
    }
}

void ADir_TaskCoordinator::CompleteTask(const FString& AgentName, const FString& Deliverables)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = EDir_TaskStatus::Completed;
            Task.RequiredDeliverables = Deliverables;
            Task.CycleCompleted = CurrentCycle;
            
            // Remove task marker
            RemoveTaskMarker(AgentName);
            
            if (bDebugMode)
            {
                UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Completed task for %s - %s"), 
                       *AgentName, *Deliverables);
            }
            break;
        }
    }
}

TArray<FDir_AgentTask> ADir_TaskCoordinator::GetTasksByPriority(EDir_AgentPriority Priority)
{
    TArray<FDir_AgentTask> FilteredTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Priority == Priority)
        {
            FilteredTasks.Add(Task);
        }
    }
    
    return FilteredTasks;
}

TArray<FDir_AgentTask> ADir_TaskCoordinator::GetTasksByStatus(EDir_TaskStatus Status)
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

void ADir_TaskCoordinator::StartNewCycle()
{
    CurrentCycle++;
    
    // Archive completed tasks
    for (int32 i = ActiveTasks.Num() - 1; i >= 0; i--)
    {
        if (ActiveTasks[i].Status == EDir_TaskStatus::Completed)
        {
            ActiveTasks.RemoveAt(i);
        }
    }
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Started Cycle %d"), CurrentCycle);
    }
}

void ADir_TaskCoordinator::EndCurrentCycle()
{
    UpdateCycleMetrics();
    
    // Log cycle completion
    FDir_CycleMetrics CurrentMetrics = GetCurrentCycleMetrics();
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Ended Cycle %d - %d/%d tasks completed"), 
               CurrentCycle, CurrentMetrics.TasksCompleted, CurrentMetrics.TasksAssigned);
    }
}

FDir_CycleMetrics ADir_TaskCoordinator::GetCurrentCycleMetrics()
{
    FDir_CycleMetrics Metrics;
    Metrics.CycleNumber = CurrentCycle;
    Metrics.TasksAssigned = ActiveTasks.Num();
    
    // Count completed tasks
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_TaskStatus::Completed)
        {
            Metrics.TasksCompleted++;
        }
    }
    
    // Check if playable prototype is ready
    Metrics.bPlayablePrototypeReady = IsPlayablePrototypeReady();
    
    return Metrics;
}

bool ADir_TaskCoordinator::IsPlayablePrototypeReady()
{
    // Check critical requirements for playable prototype
    int32 CriticalTasksCompleted = 0;
    int32 TotalCriticalTasks = 0;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Priority == EDir_AgentPriority::Critical)
        {
            TotalCriticalTasks++;
            if (Task.Status == EDir_TaskStatus::Completed)
            {
                CriticalTasksCompleted++;
            }
        }
    }
    
    // Prototype is ready when all critical tasks are completed
    return (TotalCriticalTasks > 0) && (CriticalTasksCompleted == TotalCriticalTasks);
}

void ADir_TaskCoordinator::SpawnTaskMarker(const FString& TaskName, const FVector& Location)
{
    if (UWorld* World = GetWorld())
    {
        // Spawn a simple marker actor at the task location
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("TaskMarker_%s"), *TaskName));
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AActor* MarkerActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
        
        if (MarkerActor)
        {
            MarkerActor->SetActorScale3D(FVector(0.5f));
            
            if (bDebugMode)
            {
                UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Spawned marker for %s at %s"), 
                       *TaskName, *Location.ToString());
            }
        }
    }
}

void ADir_TaskCoordinator::RemoveTaskMarker(const FString& TaskName)
{
    if (UWorld* World = GetWorld())
    {
        FString MarkerName = FString::Printf(TEXT("TaskMarker_%s"), *TaskName);
        
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetName().Contains(MarkerName))
            {
                Actor->Destroy();
                
                if (bDebugMode)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Removed marker for %s"), *TaskName);
                }
                break;
            }
        }
    }
}

void ADir_TaskCoordinator::ValidateAgentProgress()
{
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator: Validating agent progress..."));
    }
    
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        // Check if task has been pending too long
        if (Task.Status == EDir_TaskStatus::Pending && 
            (CurrentCycle - Task.CycleAssigned) > 2)
        {
            Task.Status = EDir_TaskStatus::Blocked;
            
            if (bDebugMode)
            {
                UE_LOG(LogTemp, Error, TEXT("Dir_TaskCoordinator: Task blocked - %s"), 
                       *Task.AgentName);
            }
        }
    }
}

void ADir_TaskCoordinator::UpdateCycleMetrics()
{
    FDir_CycleMetrics CurrentMetrics = GetCurrentCycleMetrics();
    
    // Update or add to cycle history
    bool bFoundExisting = false;
    for (FDir_CycleMetrics& Metrics : CycleHistory)
    {
        if (Metrics.CycleNumber == CurrentCycle)
        {
            Metrics = CurrentMetrics;
            bFoundExisting = true;
            break;
        }
    }
    
    if (!bFoundExisting)
    {
        CycleHistory.Add(CurrentMetrics);
    }
}

bool ADir_TaskCoordinator::ValidateMinPlayableMap()
{
    // Basic validation of MinPlayableMap requirements
    if (UWorld* World = GetWorld())
    {
        // Check for essential actors
        bool bHasLandscape = false;
        bool bHasCharacter = false;
        bool bHasLighting = false;
        
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor)
            {
                FString ActorName = Actor->GetClass()->GetName();
                
                if (ActorName.Contains(TEXT("Landscape")))
                {
                    bHasLandscape = true;
                }
                else if (ActorName.Contains(TEXT("TranspersonalCharacter")))
                {
                    bHasCharacter = true;
                }
                else if (ActorName.Contains(TEXT("Light")))
                {
                    bHasLighting = true;
                }
            }
        }
        
        return bHasLandscape && bHasCharacter && bHasLighting;
    }
    
    return false;
}

void ADir_TaskCoordinator::LogTaskProgress()
{
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== TASK PROGRESS REPORT - CYCLE %d ==="), CurrentCycle);
        
        for (const FDir_AgentTask& Task : ActiveTasks)
        {
            FString StatusString;
            switch (Task.Status)
            {
                case EDir_TaskStatus::Pending: StatusString = TEXT("PENDING"); break;
                case EDir_TaskStatus::InProgress: StatusString = TEXT("IN PROGRESS"); break;
                case EDir_TaskStatus::Completed: StatusString = TEXT("COMPLETED"); break;
                case EDir_TaskStatus::Failed: StatusString = TEXT("FAILED"); break;
                case EDir_TaskStatus::Blocked: StatusString = TEXT("BLOCKED"); break;
            }
            
            UE_LOG(LogTemp, Warning, TEXT("%s: %s - %s"), 
                   *Task.AgentName, *StatusString, *Task.TaskDescription);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("=== END TASK REPORT ==="));
    }
}

void ADir_TaskCoordinator::DebugPrintAllTasks()
{
    LogTaskProgress();
}

void ADir_TaskCoordinator::DebugValidatePlayableState()
{
    bool bIsReady = IsPlayablePrototypeReady();
    bool bMapValid = ValidateMinPlayableMap();
    
    UE_LOG(LogTemp, Warning, TEXT("Dir_TaskCoordinator Debug:"));
    UE_LOG(LogTemp, Warning, TEXT("- Playable Prototype Ready: %s"), bIsReady ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("- MinPlayableMap Valid: %s"), bMapValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("- Current Cycle: %d"), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("- Active Tasks: %d"), ActiveTasks.Num());
}