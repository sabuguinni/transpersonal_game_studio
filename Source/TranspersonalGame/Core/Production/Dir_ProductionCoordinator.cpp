#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    CurrentPhase = EDir_ProductionPhase::Prototype;
    CycleTimeTarget = 300.0f; // 5 minutes per cycle
    CurrentCycle = 0;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentTasks();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized - Phase: %s"), *GetPhaseDescription());
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update metrics every tick
    UpdateProductionMetrics();
    
    // Check if we can advance phase
    if (CanAdvancePhase())
    {
        UE_LOG(LogTemp, Warning, TEXT("Production phase advancement available"));
    }
}

void UDir_ProductionCoordinator::AddAgentTask(const FString& AgentName, const FString& TaskDescription, float Priority, int32 EstimatedCycles)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.EstimatedCycles = EstimatedCycles;
    NewTask.Status = EDir_AgentStatus::Idle;
    NewTask.CompletedCycles = 0;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Added task for %s: %s (Priority: %.1f)"), *AgentName, *TaskDescription, Priority);
}

void UDir_ProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Log, TEXT("Updated %s status to %d"), *AgentName, (int32)NewStatus);
            break;
        }
    }
}

void UDir_ProductionCoordinator::CompleteAgentTask(const FString& AgentName)
{
    for (int32 i = AgentTasks.Num() - 1; i >= 0; i--)
    {
        if (AgentTasks[i].AgentName == AgentName && AgentTasks[i].Status == EDir_AgentStatus::Working)
        {
            AgentTasks[i].Status = EDir_AgentStatus::Completed;
            AgentTasks[i].CompletedCycles = AgentTasks[i].EstimatedCycles;
            UE_LOG(LogTemp, Warning, TEXT("Completed task for %s: %s"), *AgentName, *AgentTasks[i].TaskDescription);
            break;
        }
    }
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetPendingTasks()
{
    return GetTasksByStatus(EDir_AgentStatus::Idle);
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetTasksByStatus(EDir_AgentStatus Status)
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

void UDir_ProductionCoordinator::AdvanceProductionPhase()
{
    if (!CanAdvancePhase())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot advance production phase - requirements not met"));
        return;
    }
    
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::Prototype;
            break;
        case EDir_ProductionPhase::Prototype:
            CurrentPhase = EDir_ProductionPhase::Alpha;
            break;
        case EDir_ProductionPhase::Alpha:
            CurrentPhase = EDir_ProductionPhase::Beta;
            break;
        case EDir_ProductionPhase::Beta:
            CurrentPhase = EDir_ProductionPhase::Polish;
            break;
        case EDir_ProductionPhase::Polish:
            CurrentPhase = EDir_ProductionPhase::Release;
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced to production phase: %s"), *GetPhaseDescription());
}

bool UDir_ProductionCoordinator::CanAdvancePhase()
{
    float Progress = GetOverallProgress();
    
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            return Progress >= 0.8f; // 80% of pre-production tasks complete
        case EDir_ProductionPhase::Prototype:
            return Progress >= 0.6f && CurrentMetrics.GameplayCompleteness >= 0.3f; // Basic gameplay working
        case EDir_ProductionPhase::Alpha:
            return Progress >= 0.8f && CurrentMetrics.GameplayCompleteness >= 0.7f; // Most features complete
        case EDir_ProductionPhase::Beta:
            return Progress >= 0.9f && CurrentMetrics.PerformanceScore >= 70.0f; // Performance optimized
        case EDir_ProductionPhase::Polish:
            return Progress >= 0.95f && CurrentMetrics.PerformanceScore >= 85.0f; // Ready for release
        default:
            return false;
    }
}

FString UDir_ProductionCoordinator::GetPhaseDescription()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            return TEXT("Pre-Production: Planning and Architecture");
        case EDir_ProductionPhase::Prototype:
            return TEXT("Prototype: Core Gameplay Implementation");
        case EDir_ProductionPhase::Alpha:
            return TEXT("Alpha: Feature Complete");
        case EDir_ProductionPhase::Beta:
            return TEXT("Beta: Content Complete, Bug Fixing");
        case EDir_ProductionPhase::Polish:
            return TEXT("Polish: Final Optimization and Polish");
        case EDir_ProductionPhase::Release:
            return TEXT("Release: Ready for Launch");
        default:
            return TEXT("Unknown Phase");
    }
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count actors in the world
    CurrentMetrics.TotalActors = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.PropCount = 0;
    
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }
        
        CurrentMetrics.TotalActors++;
        
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("trex")) || 
            ActorName.Contains(TEXT("veloci")) || ActorName.Contains(TEXT("brachi")))
        {
            CurrentMetrics.DinosaurCount++;
        }
        else if (Actor->GetRootComponent() && Actor->GetRootComponent()->IsA<UStaticMeshComponent>())
        {
            CurrentMetrics.PropCount++;
        }
    }
    
    // Calculate terrain complexity (simplified)
    CurrentMetrics.TerrainComplexity = FMath::Clamp(CurrentMetrics.PropCount / 100.0f, 0.0f, 1.0f);
    
    // Calculate gameplay completeness and performance
    CalculateGameplayCompleteness();
    CalculatePerformanceScore();
}

float UDir_ProductionCoordinator::GetOverallProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    float TotalWeight = 0.0f;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        float TaskProgress = CalculateTaskProgress(Task);
        TotalProgress += TaskProgress * Task.Priority;
        TotalWeight += Task.Priority;
    }
    
    return TotalWeight > 0.0f ? TotalProgress / TotalWeight : 0.0f;
}

void UDir_ProductionCoordinator::StartNewCycle()
{
    CurrentCycle++;
    
    // Reset working tasks to idle if they've been working too long
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Task.CompletedCycles++;
            if (Task.CompletedCycles >= Task.EstimatedCycles)
            {
                Task.Status = EDir_AgentStatus::Completed;
            }
        }
    }
    
    GenerateTaskPriorities();
    
    UE_LOG(LogTemp, Warning, TEXT("Started production cycle %d"), CurrentCycle);
}

void UDir_ProductionCoordinator::GenerateTaskPriorities()
{
    // Adjust task priorities based on current phase and metrics
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            continue;
        }
        
        // Boost priority for critical systems in prototype phase
        if (CurrentPhase == EDir_ProductionPhase::Prototype)
        {
            if (Task.AgentName.Contains(TEXT("Core")) || Task.AgentName.Contains(TEXT("Character")) || Task.AgentName.Contains(TEXT("World")))
            {
                Task.Priority = FMath::Max(Task.Priority, 5.0f);
            }
        }
        
        // Boost priority for performance and polish in later phases
        if (CurrentPhase >= EDir_ProductionPhase::Beta)
        {
            if (Task.AgentName.Contains(TEXT("Performance")) || Task.AgentName.Contains(TEXT("QA")))
            {
                Task.Priority = FMath::Max(Task.Priority, 4.0f);
            }
        }
    }
}

FString UDir_ProductionCoordinator::GetNextAgentToExecute()
{
    FDir_AgentTask* HighestPriorityTask = nullptr;
    float HighestPriority = -1.0f;
    
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle && Task.Priority > HighestPriority)
        {
            HighestPriority = Task.Priority;
            HighestPriorityTask = &Task;
        }
    }
    
    if (HighestPriorityTask)
    {
        HighestPriorityTask->Status = EDir_AgentStatus::Working;
        return HighestPriorityTask->AgentName;
    }
    
    return TEXT("No agents available");
}

void UDir_ProductionCoordinator::InitializeProductionPipeline()
{
    AgentTasks.Empty();
    InitializeAgentTasks();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline initialized with %d tasks"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::AnalyzeCurrentState()
{
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATE ANALYSIS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *GetPhaseDescription());
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), GetOverallProgress() * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Props: %d"), CurrentMetrics.PropCount);
    UE_LOG(LogTemp, Warning, TEXT("Gameplay Completeness: %.1f%%"), CurrentMetrics.GameplayCompleteness * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.1f"), CurrentMetrics.PerformanceScore);
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), GetTasksByStatus(EDir_AgentStatus::Working).Num());
    UE_LOG(LogTemp, Warning, TEXT("Pending Tasks: %d"), GetPendingTasks().Num());
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    AnalyzeCurrentState();
    
    TArray<FDir_AgentTask> CompletedTasks = GetTasksByStatus(EDir_AgentStatus::Completed);
    TArray<FDir_AgentTask> WorkingTasks = GetTasksByStatus(EDir_AgentStatus::Working);
    TArray<FDir_AgentTask> PendingTasks = GetPendingTasks();
    TArray<FDir_AgentTask> BlockedTasks = GetTasksByStatus(EDir_AgentStatus::Blocked);
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d | Working: %d | Pending: %d | Blocked: %d"), 
           CompletedTasks.Num(), WorkingTasks.Num(), PendingTasks.Num(), BlockedTasks.Num());
    
    if (CanAdvancePhase())
    {
        UE_LOG(LogTemp, Warning, TEXT("READY TO ADVANCE TO NEXT PHASE"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Phase advancement requirements not met"));
    }
}

void UDir_ProductionCoordinator::InitializeAgentTasks()
{
    // Core prototype tasks
    AddAgentTask(TEXT("Engine Architect"), TEXT("Define core architecture and systems"), 5.0f, 2);
    AddAgentTask(TEXT("Core Systems"), TEXT("Implement physics, collision, and core mechanics"), 5.0f, 3);
    AddAgentTask(TEXT("World Generator"), TEXT("Create terrain and world structure"), 4.0f, 2);
    AddAgentTask(TEXT("Environment Artist"), TEXT("Populate world with vegetation and props"), 3.0f, 2);
    AddAgentTask(TEXT("Character Artist"), TEXT("Create player character and basic animations"), 4.0f, 2);
    AddAgentTask(TEXT("NPC Behavior"), TEXT("Implement basic dinosaur AI and behavior"), 4.0f, 3);
    AddAgentTask(TEXT("Combat AI"), TEXT("Create combat system and enemy AI"), 3.0f, 2);
    AddAgentTask(TEXT("Lighting"), TEXT("Set up lighting and atmosphere"), 2.0f, 1);
    AddAgentTask(TEXT("Audio"), TEXT("Implement basic audio systems"), 2.0f, 1);
    AddAgentTask(TEXT("Performance"), TEXT("Optimize for target performance"), 3.0f, 2);
    AddAgentTask(TEXT("QA"), TEXT("Test and validate systems"), 3.0f, 1);
}

void UDir_ProductionCoordinator::CalculateGameplayCompleteness()
{
    // Simple heuristic based on key systems
    float Completeness = 0.0f;
    
    // Basic world exists
    if (CurrentMetrics.TotalActors > 10)
    {
        Completeness += 0.2f;
    }
    
    // Dinosaurs present
    if (CurrentMetrics.DinosaurCount > 0)
    {
        Completeness += 0.3f;
    }
    
    // Environment populated
    if (CurrentMetrics.PropCount > 50)
    {
        Completeness += 0.2f;
    }
    
    // Tasks completed
    TArray<FDir_AgentTask> CompletedTasks = GetTasksByStatus(EDir_AgentStatus::Completed);
    if (AgentTasks.Num() > 0)
    {
        Completeness += 0.3f * (float(CompletedTasks.Num()) / float(AgentTasks.Num()));
    }
    
    CurrentMetrics.GameplayCompleteness = FMath::Clamp(Completeness, 0.0f, 1.0f);
}

void UDir_ProductionCoordinator::CalculatePerformanceScore()
{
    // Simple performance heuristic
    float Score = 100.0f;
    
    // Penalize for too many actors
    if (CurrentMetrics.TotalActors > 5000)
    {
        Score -= (CurrentMetrics.TotalActors - 5000) * 0.01f;
    }
    
    // Penalize for too many dinosaurs
    if (CurrentMetrics.DinosaurCount > 100)
    {
        Score -= (CurrentMetrics.DinosaurCount - 100) * 0.1f;
    }
    
    CurrentMetrics.PerformanceScore = FMath::Clamp(Score, 0.0f, 100.0f);
}

float UDir_ProductionCoordinator::CalculateTaskProgress(const FDir_AgentTask& Task)
{
    switch (Task.Status)
    {
        case EDir_AgentStatus::Completed:
            return 1.0f;
        case EDir_AgentStatus::Working:
            return FMath::Clamp(float(Task.CompletedCycles) / float(Task.EstimatedCycles), 0.0f, 0.9f);
        case EDir_AgentStatus::Blocked:
        case EDir_AgentStatus::Failed:
            return 0.0f;
        default:
            return 0.0f;
    }
}