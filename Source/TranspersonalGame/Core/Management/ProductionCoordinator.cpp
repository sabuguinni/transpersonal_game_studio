#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    CurrentPhase = EDir_ProductionPhase::CoreSystems;
    CycleStartTime = 0.0f;
    CurrentCycleNumber = 4;

    // Initialize default agent tasks for current cycle
    FDir_AgentTask StudioDirectorTask;
    StudioDirectorTask.AgentName = TEXT("Studio Director");
    StudioDirectorTask.TaskDescription = TEXT("Coordinate production pipeline and manage agent tasks");
    StudioDirectorTask.Status = EDir_AgentStatus::Working;
    StudioDirectorTask.Priority = 10.0f;
    StudioDirectorTask.EstimatedHours = 2.0f;
    StudioDirectorTask.CompletionPercentage = 75.0f;
    AgentTasks.Add(StudioDirectorTask);

    FDir_AgentTask WorldGenTask;
    WorldGenTask.AgentName = TEXT("Procedural World Generator");
    WorldGenTask.TaskDescription = TEXT("Create varied terrain with hills, valleys, and rivers");
    WorldGenTask.Status = EDir_AgentStatus::Idle;
    WorldGenTask.Priority = 9.0f;
    WorldGenTask.Dependencies.Add(TEXT("Studio Director"));
    WorldGenTask.EstimatedHours = 4.0f;
    WorldGenTask.CompletionPercentage = 0.0f;
    AgentTasks.Add(WorldGenTask);

    FDir_AgentTask CharacterTask;
    CharacterTask.AgentName = TEXT("Character Artist Agent");
    CharacterTask.TaskDescription = TEXT("Implement MetaHuman character variations");
    CharacterTask.Status = EDir_AgentStatus::Idle;
    CharacterTask.Priority = 8.0f;
    CharacterTask.Dependencies.Add(TEXT("Studio Director"));
    CharacterTask.EstimatedHours = 3.0f;
    CharacterTask.CompletionPercentage = 0.0f;
    AgentTasks.Add(CharacterTask);
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    CycleStartTime = GetWorld()->GetTimeSeconds();
    InitializeProductionCycle(CurrentCycleNumber);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized for Cycle %d"), CurrentCycleNumber);
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update production metrics every 5 seconds
    static float LastUpdateTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastUpdateTime > 5.0f)
    {
        UpdateProductionMetrics();
        LastUpdateTime = CurrentTime;
    }
}

void AProductionCoordinator::InitializeProductionCycle(int32 CycleNumber)
{
    CurrentCycleNumber = CycleNumber;
    CycleStartTime = GetWorld()->GetTimeSeconds();
    
    // Reset completion percentages for new cycle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            Task.Status = EDir_AgentStatus::Idle;
            Task.CompletionPercentage = 0.0f;
        }
    }
    
    UpdateProductionMetrics();
    LogProductionState();
}

void AProductionCoordinator::UpdateAgentTask(const FString& AgentName, EDir_AgentStatus NewStatus, float CompletionPercentage)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            Task.CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
            
            UE_LOG(LogTemp, Log, TEXT("Updated task for %s: Status=%d, Completion=%.1f%%"), 
                *AgentName, (int32)NewStatus, CompletionPercentage);
            break;
        }
    }
    
    UpdateProductionMetrics();
}

void AProductionCoordinator::AddAgentTask(const FDir_AgentTask& NewTask)
{
    AgentTasks.Add(NewTask);
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Added new task for %s: %s"), 
        *NewTask.AgentName, *NewTask.TaskDescription);
}

FDir_ProductionMetrics AProductionCoordinator::CalculateProductionMetrics()
{
    FDir_ProductionMetrics Metrics;
    
    // Count actors in the level
    UWorld* World = GetWorld();
    if (World)
    {
        Metrics.TotalActorsInLevel = World->GetCurrentLevel()->Actors.Num();
    }
    
    // Calculate task completion
    int32 CompletedTasks = 0;
    int32 TotalTasks = AgentTasks.Num();
    float TotalCompletion = 0.0f;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedTasks++;
        }
        TotalCompletion += Task.CompletionPercentage;
    }
    
    Metrics.CompletedTasks = CompletedTasks;
    Metrics.TotalTasks = TotalTasks;
    
    // Calculate specific system completions based on actor counts
    Metrics.TerrainCompletion = FMath::Min(100.0f, Metrics.TotalActorsInLevel * 0.1f);
    Metrics.CharacterSystemCompletion = CompletedTasks > 0 ? 50.0f : 0.0f;
    Metrics.DinosaurPopulationCompletion = FMath::Min(100.0f, Metrics.TotalActorsInLevel * 0.05f);
    
    // Overall progress
    if (TotalTasks > 0)
    {
        Metrics.OverallProgress = TotalCompletion / TotalTasks;
    }
    
    return Metrics;
}

TArray<FDir_AgentTask> AProductionCoordinator::GetPriorityTasks()
{
    TArray<FDir_AgentTask> PriorityTasks = AgentTasks;
    
    // Sort by priority (highest first)
    PriorityTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B)
    {
        return A.Priority > B.Priority;
    });
    
    // Filter only non-completed tasks
    PriorityTasks.RemoveAll([](const FDir_AgentTask& Task)
    {
        return Task.Status == EDir_AgentStatus::Completed;
    });
    
    return PriorityTasks;
}

bool AProductionCoordinator::CheckTaskDependencies(const FString& AgentName)
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            // Check if all dependencies are completed
            for (const FString& Dependency : Task.Dependencies)
            {
                bool DependencyCompleted = false;
                for (const FDir_AgentTask& DepTask : AgentTasks)
                {
                    if (DepTask.AgentName == Dependency && DepTask.Status == EDir_AgentStatus::Completed)
                    {
                        DependencyCompleted = true;
                        break;
                    }
                }
                
                if (!DependencyCompleted)
                {
                    return false;
                }
            }
            return true;
        }
    }
    
    return false;
}

void AProductionCoordinator::AdvanceProductionPhase()
{
    int32 CurrentPhaseInt = (int32)CurrentPhase;
    CurrentPhaseInt++;
    
    if (CurrentPhaseInt <= (int32)EDir_ProductionPhase::Testing)
    {
        CurrentPhase = (EDir_ProductionPhase)CurrentPhaseInt;
        UE_LOG(LogTemp, Warning, TEXT("Advanced to production phase: %s"), *GetCurrentPhaseString());
    }
}

void AProductionCoordinator::GenerateProductionReport()
{
    ProductionMetrics = CalculateProductionMetrics();
    
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT - CYCLE %d ===\n"), CurrentCycleNumber);
    Report += FString::Printf(TEXT("Current Phase: %s\n"), *GetCurrentPhaseString());
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), ProductionMetrics.OverallProgress);
    Report += FString::Printf(TEXT("Completed Tasks: %d/%d\n"), ProductionMetrics.CompletedTasks, ProductionMetrics.TotalTasks);
    Report += FString::Printf(TEXT("Total Actors: %d\n"), ProductionMetrics.TotalActorsInLevel);
    Report += FString::Printf(TEXT("Terrain Completion: %.1f%%\n"), ProductionMetrics.TerrainCompletion);
    Report += FString::Printf(TEXT("Character System: %.1f%%\n"), ProductionMetrics.CharacterSystemCompletion);
    Report += FString::Printf(TEXT("Dinosaur Population: %.1f%%\n"), ProductionMetrics.DinosaurPopulationCompletion);
    
    Report += TEXT("\n=== AGENT TASKS ===\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusStr;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusStr = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusStr = TEXT("WORKING"); break;
            case EDir_AgentStatus::Completed: StatusStr = TEXT("COMPLETED"); break;
            case EDir_AgentStatus::Blocked: StatusStr = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Failed: StatusStr = TEXT("FAILED"); break;
        }
        
        Report += FString::Printf(TEXT("%s [%s] %.1f%% - %s\n"), 
            *Task.AgentName, *StatusStr, Task.CompletionPercentage, *Task.TaskDescription);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void AProductionCoordinator::ResetProductionState()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.CompletionPercentage = 0.0f;
    }
    
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    CycleStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Production state reset"));
}

float AProductionCoordinator::GetOverallProgress() const
{
    return ProductionMetrics.OverallProgress;
}

int32 AProductionCoordinator::GetCompletedTaskCount() const
{
    return ProductionMetrics.CompletedTasks;
}

FString AProductionCoordinator::GetCurrentPhaseString() const
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction: return TEXT("Pre-Production");
        case EDir_ProductionPhase::CoreSystems: return TEXT("Core Systems");
        case EDir_ProductionPhase::WorldBuilding: return TEXT("World Building");
        case EDir_ProductionPhase::CharacterSystems: return TEXT("Character Systems");
        case EDir_ProductionPhase::GameplayMechanics: return TEXT("Gameplay Mechanics");
        case EDir_ProductionPhase::Polish: return TEXT("Polish");
        case EDir_ProductionPhase::Testing: return TEXT("Testing");
        default: return TEXT("Unknown");
    }
}

void AProductionCoordinator::UpdateProductionMetrics()
{
    ProductionMetrics = CalculateProductionMetrics();
}

void AProductionCoordinator::LogProductionState()
{
    UE_LOG(LogTemp, Log, TEXT("Production Cycle %d - Phase: %s - Progress: %.1f%%"), 
        CurrentCycleNumber, *GetCurrentPhaseString(), ProductionMetrics.OverallProgress);
}

void AProductionCoordinator::ValidateAgentDependencies()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            if (CheckTaskDependencies(Task.AgentName))
            {
                Task.Status = EDir_AgentStatus::Idle;
                UE_LOG(LogTemp, Log, TEXT("Unblocked task for %s - dependencies satisfied"), *Task.AgentName);
            }
        }
    }
}