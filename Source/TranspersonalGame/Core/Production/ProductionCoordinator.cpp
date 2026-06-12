#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    bAutoUpdateMetrics = true;
    MetricsUpdateInterval = 30.0f; // Update every 30 seconds
    
    // Initialize default metrics
    CurrentMetrics = FDir_ProductionMetrics();
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default tasks for all agents
    InitializeDefaultTasks();
    
    // Start automatic metrics updates if enabled
    if (bAutoUpdateMetrics)
    {
        GetWorldTimerManager().SetTimer(MetricsUpdateTimer, this, 
            &AProductionCoordinator::UpdateMetricsTimer, MetricsUpdateInterval, true);
    }
    
    // Initial metrics calculation
    UpdateProductionMetrics();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, 
                                             const FString& ExpectedDeliverable, EDir_AgentPriority Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.ExpectedDeliverable = ExpectedDeliverable;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_TaskStatus::PENDING;
    NewTask.AssignedTime = FDateTime::Now();
    
    // Estimate hours based on priority and complexity
    switch (Priority)
    {
        case EDir_AgentPriority::CRITICAL:
            NewTask.EstimatedHours = 4.0f;
            break;
        case EDir_AgentPriority::HIGH:
            NewTask.EstimatedHours = 2.0f;
            break;
        case EDir_AgentPriority::MEDIUM:
            NewTask.EstimatedHours = 1.0f;
            break;
        case EDir_AgentPriority::LOW:
            NewTask.EstimatedHours = 0.5f;
            break;
    }
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription);
}

void AProductionCoordinator::UpdateTaskStatus(const FString& AgentName, EDir_TaskStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentName == AgentName && Task.Status != EDir_TaskStatus::COMPLETED)
        {
            Task.Status = NewStatus;
            
            if (NewStatus == EDir_TaskStatus::COMPLETED)
            {
                Task.CompletedTime = FDateTime::Now();
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Task status updated for %s: %d"), *AgentName, (int32)NewStatus);
            break;
        }
    }
}

void AProductionCoordinator::CompleteTask(const FString& AgentName)
{
    UpdateTaskStatus(AgentName, EDir_TaskStatus::COMPLETED);
    UpdateProductionMetrics();
}

TArray<FDir_AgentTask> AProductionCoordinator::GetTasksByPriority(EDir_AgentPriority Priority)
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

TArray<FDir_AgentTask> AProductionCoordinator::GetTasksByStatus(EDir_TaskStatus Status)
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

void AProductionCoordinator::UpdateProductionMetrics()
{
    AnalyzeMapActors();
    CalculateTaskProgress();
    
    CurrentMetrics.Milestone1Progress = CalculateMilestone1Progress();
    CurrentMetrics.OverallProductionHealth = CalculateProductionHealth();
    CurrentMetrics.LastUpdateTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("Production metrics updated - Milestone 1: %.1f%%, Health: %.1f%%"), 
           CurrentMetrics.Milestone1Progress, CurrentMetrics.OverallProductionHealth);
}

float AProductionCoordinator::CalculateMilestone1Progress()
{
    // Milestone 1 requirements and their weights
    float TotalWeight = 6.0f;
    float CompletedWeight = 0.0f;
    
    // Check each milestone requirement
    UWorld* World = GetWorld();
    if (!World) return 0.0f;
    
    // 1. Playable Character (Weight: 1.0)
    bool bHasPlayableCharacter = false;
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (ActorItr->IsA<APawn>() && ActorItr->GetName().Contains(TEXT("Character")))
        {
            bHasPlayableCharacter = true;
            break;
        }
    }
    if (bHasPlayableCharacter) CompletedWeight += 1.0f;
    
    // 2. Terrain with height variation (Weight: 1.0)
    bool bHasLandscape = CountActorsByClass(ALandscapeProxy::StaticClass()) > 0;
    if (bHasLandscape) CompletedWeight += 1.0f;
    
    // 3. Dinosaur actors (Weight: 1.0)
    int32 DinosaurCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetName().Contains(TEXT("Dinosaur")) || 
            ActorItr->GetName().Contains(TEXT("TRex")) ||
            ActorItr->GetName().Contains(TEXT("Raptor")) ||
            ActorItr->GetName().Contains(TEXT("Brachio")))
        {
            DinosaurCount++;
        }
    }
    if (DinosaurCount >= 3) CompletedWeight += 1.0f;
    
    // 4. Movement system (Weight: 1.0) - Assume functional if character exists
    if (bHasPlayableCharacter) CompletedWeight += 1.0f;
    
    // 5. Camera system (Weight: 1.0) - Assume functional if character exists  
    if (bHasPlayableCharacter) CompletedWeight += 1.0f;
    
    // 6. Lighting system (Weight: 1.0)
    bool bHasLighting = CountActorsByClass(ADirectionalLight::StaticClass()) > 0;
    if (bHasLighting) CompletedWeight += 1.0f;
    
    return (CompletedWeight / TotalWeight) * 100.0f;
}

float AProductionCoordinator::CalculateProductionHealth()
{
    float Health = 100.0f;
    
    // Deduct points for blocked tasks
    int32 BlockedTasks = GetTasksByStatus(EDir_TaskStatus::BLOCKED).Num();
    Health -= (BlockedTasks * 10.0f);
    
    // Deduct points for failed tasks
    int32 FailedTasks = GetTasksByStatus(EDir_TaskStatus::FAILED).Num();
    Health -= (FailedTasks * 15.0f);
    
    // Deduct points for overdue tasks (simplified - assume tasks over 24 hours are overdue)
    FDateTime Now = FDateTime::Now();
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_TaskStatus::PENDING || Task.Status == EDir_TaskStatus::IN_PROGRESS)
        {
            FTimespan TimeSinceAssigned = Now - Task.AssignedTime;
            if (TimeSinceAssigned.GetTotalHours() > 24.0)
            {
                Health -= 5.0f;
            }
        }
    }
    
    return FMath::Clamp(Health, 0.0f, 100.0f);
}

int32 AProductionCoordinator::CountActorsByClass(UClass* ActorClass)
{
    int32 Count = 0;
    UWorld* World = GetWorld();
    
    if (World && ActorClass)
    {
        for (TActorIterator<AActor> ActorItr(World, ActorClass); ActorItr; ++ActorItr)
        {
            Count++;
        }
    }
    
    return Count;
}

void AProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Active Tasks: %d"), ActiveTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), CurrentMetrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Pending Tasks: %d"), CurrentMetrics.PendingTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked Tasks: %d"), CurrentMetrics.BlockedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: %.1f%%"), CurrentMetrics.Milestone1Progress);
    UE_LOG(LogTemp, Warning, TEXT("Production Health: %.1f%%"), CurrentMetrics.OverallProductionHealth);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors in Map: %d"), CurrentMetrics.TotalActorsInMap);
}

void AProductionCoordinator::InitializeDefaultTasks()
{
    // Clear existing tasks
    ActiveTasks.Empty();
    
    // Assign critical tasks for Milestone 1
    AssignTaskToAgent(TEXT("Agent_02_Engine_Architect"), 
                     TEXT("Validate SharedTypes.h compilation and fix cross-module dependencies"),
                     TEXT("Clean compilation of all existing .h/.cpp files"), 
                     EDir_AgentPriority::HIGH);
    
    AssignTaskToAgent(TEXT("Agent_05_World_Generator"), 
                     TEXT("Generate realistic terrain with height variation using PCG"),
                     TEXT("Replace flat terrain with hills, valleys, and natural landscape"), 
                     EDir_AgentPriority::CRITICAL);
    
    AssignTaskToAgent(TEXT("Agent_09_Character_Artist"), 
                     TEXT("Create TranspersonalCharacter with proper mesh and animations"),
                     TEXT("Playable character with walking/running animations"), 
                     EDir_AgentPriority::HIGH);
    
    AssignTaskToAgent(TEXT("Agent_10_Animation"), 
                     TEXT("Implement basic character movement animations and IK"),
                     TEXT("Smooth character movement with foot IK on terrain"), 
                     EDir_AgentPriority::HIGH);
    
    AssignTaskToAgent(TEXT("Agent_12_Combat_AI"), 
                     TEXT("Create basic dinosaur AI with territorial behavior"),
                     TEXT("Dinosaur actors that patrol and react to player presence"), 
                     EDir_AgentPriority::MEDIUM);
}

void AProductionCoordinator::RefreshMetricsNow()
{
    UpdateProductionMetrics();
    GenerateProductionReport();
}

void AProductionCoordinator::ValidateMapState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found for map validation"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== MAP STATE VALIDATION ==="));
    
    // Count all actors
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total actors in map: %d"), TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), CountActorsByClass(APawn::StaticClass()));
    UE_LOG(LogTemp, Warning, TEXT("Lights: %d"), CountActorsByClass(ALight::StaticClass()));
    UE_LOG(LogTemp, Warning, TEXT("Static Meshes: %d"), CountActorsByClass(AStaticMeshActor::StaticClass()));
}

void AProductionCoordinator::UpdateMetricsTimer()
{
    UpdateProductionMetrics();
}

void AProductionCoordinator::AnalyzeMapActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    CurrentMetrics.TotalActorsInMap = TotalActors;
}

void AProductionCoordinator::CalculateTaskProgress()
{
    CurrentMetrics.CompletedTasks = GetTasksByStatus(EDir_TaskStatus::COMPLETED).Num();
    CurrentMetrics.PendingTasks = GetTasksByStatus(EDir_TaskStatus::PENDING).Num();
    CurrentMetrics.BlockedTasks = GetTasksByStatus(EDir_TaskStatus::BLOCKED).Num();
}