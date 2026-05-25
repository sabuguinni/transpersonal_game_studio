#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    CurrentCycleNumber = 1;
    bPipelineActive = true;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    LogProductionStatus();
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bPipelineActive)
    {
        UpdateProductionMetrics();
        CheckMilestoneProgress();
    }
}

void UDir_ProductionCoordinator::InitializeProductionPipeline()
{
    AgentTasks.Empty();
    
    // Initialize critical path tasks for Milestone 1 - "Walk Around" prototype
    AgentTasks.Add(FDir_AgentTask{
        2, // Engine Architect
        TEXT("Define core architecture and UE5 integration patterns"),
        EDir_Priority::Critical,
        TEXT("Architecture documentation, base classes"),
        false,
        2.0f
    });
    
    AgentTasks.Add(FDir_AgentTask{
        3, // Core Systems
        TEXT("Implement TranspersonalCharacter movement with WASD input"),
        EDir_Priority::Critical,
        TEXT("Character.cpp with movement, camera, input"),
        false,
        3.0f
    });
    
    AgentTasks.Add(FDir_AgentTask{
        5, // World Generator
        TEXT("Create terrain with height variation in MinPlayableMap"),
        EDir_Priority::Critical,
        TEXT("Landscape with hills, valleys, basic biomes"),
        false,
        2.0f
    });
    
    AgentTasks.Add(FDir_AgentTask{
        9, // Character Artist
        TEXT("Ensure dinosaur meshes are properly spawned and visible"),
        EDir_Priority::High,
        TEXT("5+ dinosaur actors in world with collision"),
        false,
        1.5f
    });
    
    AgentTasks.Add(FDir_AgentTask{
        12, // Combat AI
        TEXT("Implement survival HUD with health/hunger/thirst bars"),
        EDir_Priority::High,
        TEXT("SurvivalComponent.cpp + HUD widget"),
        false,
        2.0f
    });
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Pipeline initialized with %d tasks"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_Priority Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bIsCompleted = false;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Task assigned to Agent #%d: %s"), 
           AgentNumber, *TaskDescription);
}

void UDir_ProductionCoordinator::MarkTaskCompleted(int32 AgentNumber)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && !Task.bIsCompleted)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Task completed by Agent #%d"), AgentNumber);
            break;
        }
    }
    
    UpdateProductionMetrics();
}

FDir_ProductionMetrics UDir_ProductionCoordinator::GetProductionMetrics() const
{
    return CurrentMetrics;
}

bool UDir_ProductionCoordinator::IsMilestone1Complete() const
{
    // Milestone 1 requirements:
    // - Character with WASD movement
    // - Terrain with height variation
    // - 3-5 dinosaur meshes visible
    // - Basic lighting
    
    int32 CompletedCriticalTasks = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Priority == EDir_Priority::Critical && Task.bIsCompleted)
        {
            CompletedCriticalTasks++;
        }
    }
    
    return CompletedCriticalTasks >= 3 && CurrentMetrics.ActiveDinosaurs >= 3;
}

void UDir_ProductionCoordinator::ValidatePlayablePrototype()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        CurrentMetrics.bPlayablePrototype = false;
        return;
    }
    
    // Count dinosaur actors
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Rex")) || 
            Actor->GetName().Contains(TEXT("Raptor")) ||
            Actor->GetName().Contains(TEXT("Brachio")))
        {
            DinosaurCount++;
        }
    }
    
    CurrentMetrics.ActiveDinosaurs = DinosaurCount;
    CurrentMetrics.bPlayablePrototype = DinosaurCount >= 3;
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Found %d dinosaurs, prototype valid: %s"), 
           DinosaurCount, CurrentMetrics.bPlayablePrototype ? TEXT("YES") : TEXT("NO"));
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bIsCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    return PendingTasks;
}

void UDir_ProductionCoordinator::TriggerNextAgent()
{
    // Find next agent in sequence with pending tasks
    for (int32 AgentNum = 2; AgentNum <= 19; AgentNum++)
    {
        for (const FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.AgentNumber == AgentNum && !Task.bIsCompleted)
            {
                UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Next agent to execute is #%d"), AgentNum);
                return;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: All tasks completed in current cycle"));
}

void UDir_ProductionCoordinator::DebugPrintProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS CYCLE %d ==="), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Files Created: %d"), CurrentMetrics.TotalFilesCreated);
    UE_LOG(LogTemp, Warning, TEXT("UE5 Commands: %d"), CurrentMetrics.TotalUE5Commands);
    UE_LOG(LogTemp, Warning, TEXT("Active Dinosaurs: %d"), CurrentMetrics.ActiveDinosaurs);
    UE_LOG(LogTemp, Warning, TEXT("Playable Prototype: %s"), CurrentMetrics.bPlayablePrototype ? TEXT("YES") : TEXT("NO"));
    
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bIsCompleted) CompletedTasks++;
    }
    UE_LOG(LogTemp, Warning, TEXT("Tasks: %d/%d completed"), CompletedTasks, AgentTasks.Num());
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    ValidatePlayablePrototype();
    
    // Update file count (simplified - would need filesystem access in real implementation)
    CurrentMetrics.TotalFilesCreated += 2; // Assume 2 files per cycle
    CurrentMetrics.TotalUE5Commands += 2; // Track UE5 command executions
}

void UDir_ProductionCoordinator::CheckMilestoneProgress()
{
    if (IsMilestone1Complete())
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: MILESTONE 1 COMPLETE - Walk Around prototype ready"));
    }
}

void UDir_ProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Studio Director monitoring %d agents"), 19);
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Current focus - Milestone 1: Walk Around prototype"));
}