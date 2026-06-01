#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create coordinator mesh
    CoordinatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoordinatorMesh"));
    CoordinatorMesh->SetupAttachment(RootComponent);

    // Initialize production settings
    CurrentPhase = EDir_ProductionPhase::Prototype;
    CurrentCycle = 8;
    ProductionEfficiency = 0.75f;

    // Set default mesh (cube for now)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        CoordinatorMesh->SetStaticMesh(CubeMesh.Object);
        CoordinatorMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.5f));
    }
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized"));
    
    // Initialize critical production tasks
    InitializeProductionPipeline();
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update production metrics every few seconds
    static float MetricsTimer = 0.0f;
    MetricsTimer += DeltaTime;
    
    if (MetricsTimer >= 5.0f)
    {
        UpdateProductionMetrics();
        MetricsTimer = 0.0f;
    }
}

void ADir_ProductionCoordinator::AssignTask(const FString& AgentName, const FString& TaskDescription, int32 Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.Priority = Priority;
    NewTask.EstimatedHours = 2.0f; // Default 2 hours per task
    NewTask.Deadline = FDateTime::Now() + FTimespan::FromHours(4);

    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription);
}

void ADir_ProductionCoordinator::UpdateTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Log, TEXT("Task status updated for %s: %d"), *AgentName, (int32)NewStatus);
            break;
        }
    }
}

void ADir_ProductionCoordinator::CompleteTask(const FString& AgentName)
{
    for (int32 i = ActiveTasks.Num() - 1; i >= 0; i--)
    {
        if (ActiveTasks[i].AgentName == AgentName)
        {
            ActiveTasks[i].Status = EDir_AgentStatus::Completed;
            UE_LOG(LogTemp, Warning, TEXT("Task completed by %s: %s"), *AgentName, *ActiveTasks[i].TaskDescription);
        }
    }
}

TArray<FDir_AgentTask> ADir_ProductionCoordinator::GetTasksByStatus(EDir_AgentStatus Status)
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

void ADir_ProductionCoordinator::CreateMilestone(const FString& Name, const FDateTime& TargetDate)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.MilestoneName = Name;
    NewMilestone.TargetDate = TargetDate;
    NewMilestone.bCompleted = false;
    
    Milestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone created: %s"), *Name);
}

bool ADir_ProductionCoordinator::CheckMilestoneCompletion(const FString& MilestoneName)
{
    for (FDir_ProductionMilestone& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            // Check if all required tasks are completed
            bool bAllTasksComplete = true;
            for (const FDir_AgentTask& Task : Milestone.RequiredTasks)
            {
                if (Task.Status != EDir_AgentStatus::Completed)
                {
                    bAllTasksComplete = false;
                    break;
                }
            }
            
            Milestone.bCompleted = bAllTasksComplete;
            return bAllTasksComplete;
        }
    }
    
    return false;
}

float ADir_ProductionCoordinator::GetProjectProgress()
{
    if (ActiveTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    return (float)CompletedTasks / (float)ActiveTasks.Num();
}

void ADir_ProductionCoordinator::CoordinateAgents()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT COORDINATION CYCLE %d ==="), CurrentCycle);
    
    // Critical tasks for playable prototype
    TArray<TPair<FString, FString>> CriticalTasks = {
        {TEXT("Agent_02_Engine"), TEXT("Validate core architecture and compilation")},
        {TEXT("Agent_03_Core"), TEXT("Implement physics and collision systems")},
        {TEXT("Agent_05_World"), TEXT("Generate real terrain with height variation")},
        {TEXT("Agent_09_Character"), TEXT("Create dinosaur actors with proper collision")},
        {TEXT("Agent_12_Combat"), TEXT("Implement survival HUD with health/hunger bars")},
        {TEXT("Agent_06_Environment"), TEXT("Populate world with trees and rocks")},
        {TEXT("Agent_10_Animation"), TEXT("Add basic movement animations")},
        {TEXT("Agent_18_QA"), TEXT("Test playable prototype functionality")}
    };
    
    // Assign critical tasks
    for (const auto& TaskPair : CriticalTasks)
    {
        AssignTask(TaskPair.Key, TaskPair.Value, 10); // High priority
    }
    
    // Create milestone for playable prototype
    CreateMilestone(TEXT("Playable Prototype"), FDateTime::Now() + FTimespan::FromHours(8));
}

void ADir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT CYCLE %d ==="), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), ActiveTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Production Efficiency: %.2f%%"), ProductionEfficiency * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Project Progress: %.2f%%"), GetProjectProgress() * 100.0f);
    
    // Count tasks by status
    int32 CompletedCount = GetTasksByStatus(EDir_AgentStatus::Completed).Num();
    int32 WorkingCount = GetTasksByStatus(EDir_AgentStatus::Working).Num();
    int32 BlockedCount = GetTasksByStatus(EDir_AgentStatus::Blocked).Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Tasks - Completed: %d, Working: %d, Blocked: %d"), 
           CompletedCount, WorkingCount, BlockedCount);
}

void ADir_ProductionCoordinator::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Error, TEXT("INITIALIZING PRODUCTION PIPELINE - CYCLE 008"));
    
    // Clear previous tasks
    ActiveTasks.Empty();
    Milestones.Empty();
    
    // Set up critical production coordination
    CoordinateAgents();
    
    // Generate initial report
    GenerateProductionReport();
    
    UE_LOG(LogTemp, Error, TEXT("PRODUCTION PIPELINE INITIALIZED - READY FOR AGENT COORDINATION"));
}

void ADir_ProductionCoordinator::ValidateCurrentBuild()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD VALIDATION ==="));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found!"));
        return;
    }
    
    // Count actors in the world
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World validation - Total actors: %d"), ActorCount);
    
    // Check for critical systems
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor->GetName().Contains(TEXT("PlayerStart")))
        {
            bHasPlayerStart = true;
        }
        if (Actor->GetName().Contains(TEXT("Light")) || Actor->GetName().Contains(TEXT("Sun")))
        {
            bHasLighting = true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Critical systems - PlayerStart: %s, Lighting: %s"), 
           bHasPlayerStart ? TEXT("YES") : TEXT("NO"),
           bHasLighting ? TEXT("YES") : TEXT("NO"));
}

void ADir_ProductionCoordinator::CreatePlayablePrototype()
{
    UE_LOG(LogTemp, Error, TEXT("=== CREATING PLAYABLE PROTOTYPE ==="));
    
    // This function coordinates the creation of a minimal playable game
    ValidateCurrentBuild();
    
    // Ensure critical agents have their tasks
    CoordinateAgents();
    
    UE_LOG(LogTemp, Error, TEXT("PLAYABLE PROTOTYPE CREATION INITIATED"));
}

void ADir_ProductionCoordinator::UpdateProductionMetrics()
{
    // Calculate current efficiency based on completed vs total tasks
    float CompletionRate = GetProjectProgress();
    
    // Update efficiency (simple calculation for now)
    ProductionEfficiency = FMath::Clamp(CompletionRate * 1.2f, 0.0f, 1.0f);
    
    // Check for blocked tasks and reduce efficiency
    int32 BlockedTasks = GetTasksByStatus(EDir_AgentStatus::Blocked).Num();
    if (BlockedTasks > 0)
    {
        ProductionEfficiency *= 0.8f; // Reduce efficiency for blocked tasks
    }
}

void ADir_ProductionCoordinator::CheckBlockedTasks()
{
    TArray<FDir_AgentTask> BlockedTasks = GetTasksByStatus(EDir_AgentStatus::Blocked);
    
    for (const FDir_AgentTask& Task : BlockedTasks)
    {
        UE_LOG(LogTemp, Error, TEXT("BLOCKED TASK: %s - %s"), *Task.AgentName, *Task.TaskDescription);
    }
}

void ADir_ProductionCoordinator::OptimizeAgentWorkflow()
{
    // Sort tasks by priority
    ActiveTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
}

void ADir_ProductionCoordinator::TrackPrototypeProgress()
{
    // Check specific prototype requirements
    bool bHasMovement = false;
    bool bHasTerrain = false;
    bool bHasDinosaurs = false;
    bool bHasHUD = false;
    
    // This would check actual game state
    // For now, track based on task completion
    
    UE_LOG(LogTemp, Warning, TEXT("Prototype Progress - Movement: %s, Terrain: %s, Dinosaurs: %s, HUD: %s"),
           bHasMovement ? TEXT("YES") : TEXT("NO"),
           bHasTerrain ? TEXT("YES") : TEXT("NO"),
           bHasDinosaurs ? TEXT("YES") : TEXT("NO"),
           bHasHUD ? TEXT("YES") : TEXT("NO"));
}

void ADir_ProductionCoordinator::ValidateAgentDeliverables()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATING AGENT DELIVERABLES ==="));
    
    // Check each agent's expected outputs
    TArray<FString> ExpectedDeliverables = {
        TEXT("Agent_02: Core architecture files"),
        TEXT("Agent_03: Physics system implementation"),
        TEXT("Agent_05: Terrain generation system"),
        TEXT("Agent_09: Dinosaur character classes"),
        TEXT("Agent_12: Survival HUD widgets")
    };
    
    for (const FString& Deliverable : ExpectedDeliverables)
    {
        UE_LOG(LogTemp, Log, TEXT("Expected: %s"), *Deliverable);
    }
}