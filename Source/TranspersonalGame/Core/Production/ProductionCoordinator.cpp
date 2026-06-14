#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"

UProductionCoordinator::UProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    ProductionUpdateInterval = 30.0f; // Update every 30 seconds
    
    // Initialize default production metrics
    CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Prototype;
    CurrentMetrics.CompletionPercentage = 15.0f; // Current prototype state
}

void UProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Start production monitoring timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ProductionTimerHandle,
            this,
            &UProductionCoordinator::UpdateProductionTimer,
            ProductionUpdateInterval,
            true
        );
    }
    
    // Initialize the production pipeline
    InitializeProductionPipeline();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: System initialized and monitoring started"));
}

void UProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Lightweight per-frame updates
    CoordinateAgentPriorities();
}

void UProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.StartTime = FDateTime::Now();
    NewTask.EstimatedCompletion = FDateTime::Now() + FTimespan::FromMinutes(30); // Default 30min estimate
    
    // Remove existing task for this agent if any
    AgentTasks.RemoveAll([&AgentName](const FDir_AgentTask& Task) {
        return Task.AgentName == AgentName;
    });
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Assigned task '%s' to agent '%s' with priority %.2f"), 
           *TaskDescription, *AgentName, Priority);
}

void UProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            
            if (NewStatus == EDir_AgentStatus::Completed)
            {
                UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent '%s' completed task: %s"), 
                       *AgentName, *Task.TaskDescription);
            }
            break;
        }
    }
}

void UProductionCoordinator::UpdateProductionMetrics()
{
    AnalyzeWorldState();
    
    // Calculate completion percentage based on current world state
    float BaseCompletion = 15.0f; // Base prototype completion
    
    if (CurrentMetrics.CharacterCount > 0) BaseCompletion += 10.0f;
    if (CurrentMetrics.DinosaurCount >= 5) BaseCompletion += 15.0f;
    if (CurrentMetrics.TerrainActors > 0) BaseCompletion += 10.0f;
    if (CurrentMetrics.TotalActors > 100) BaseCompletion += 5.0f;
    
    // Count completed agent tasks
    int32 CompletedTasks = AgentTasks.Num() > 0 ? 
        AgentTasks.FilterByPredicate([](const FDir_AgentTask& Task) {
            return Task.Status == EDir_AgentStatus::Completed;
        }).Num() : 0;
    
    if (AgentTasks.Num() > 0)
    {
        float TaskCompletion = (float)CompletedTasks / AgentTasks.Num() * 20.0f;
        BaseCompletion += TaskCompletion;
    }
    
    CurrentMetrics.CompletionPercentage = FMath::Clamp(BaseCompletion, 0.0f, 100.0f);
    
    // Update production phase based on completion
    if (CurrentMetrics.CompletionPercentage >= 80.0f)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Beta;
    }
    else if (CurrentMetrics.CompletionPercentage >= 50.0f)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Alpha;
    }
    else if (CurrentMetrics.CompletionPercentage >= 25.0f)
    {
        CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Prototype;
    }
}

FDir_ProductionMetrics UProductionCoordinator::GetProductionMetrics() const
{
    return CurrentMetrics;
}

TArray<FDir_AgentTask> UProductionCoordinator::GetAgentTasks() const
{
    return AgentTasks;
}

void UProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentMetrics.CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Production phase changed to %d"), (int32)NewPhase);
}

void UProductionCoordinator::InitializeProductionPipeline()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Assign initial tasks to key agents for prototype phase
    AssignTaskToAgent(TEXT("Engine Architect"), TEXT("Optimize core systems for 60fps target"), 9.0f);
    AssignTaskToAgent(TEXT("Procedural World Generator"), TEXT("Generate varied terrain with biomes"), 8.5f);
    AssignTaskToAgent(TEXT("Character Artist"), TEXT("Create playable character with survival stats"), 8.0f);
    AssignTaskToAgent(TEXT("Animation Agent"), TEXT("Implement character movement animations"), 7.5f);
    AssignTaskToAgent(TEXT("NPC Behavior Agent"), TEXT("Create basic dinosaur AI behaviors"), 7.0f);
    AssignTaskToAgent(TEXT("Combat & Enemy AI"), TEXT("Implement dinosaur combat mechanics"), 6.5f);
    AssignTaskToAgent(TEXT("Environment Artist"), TEXT("Populate world with props and vegetation"), 6.0f);
    AssignTaskToAgent(TEXT("Lighting & Atmosphere"), TEXT("Setup dynamic day/night cycle"), 5.5f);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Production pipeline initialized with %d agent tasks"), AgentTasks.Num());
}

void UProductionCoordinator::GenerateProductionReport()
{
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentMetrics.CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Completion: %.1f%%"), CurrentMetrics.CompletionPercentage);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), CurrentMetrics.CharacterCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Terrain: %d"), CurrentMetrics.TerrainActors);
    
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT STATUS ==="));
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
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s [%s] Priority: %.1f"), 
               *Task.AgentName, *Task.TaskDescription, *StatusStr, Task.Priority);
    }
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UProductionCoordinator::UpdateProductionTimer()
{
    UpdateProductionMetrics();
    GenerateProductionReport();
}

void UProductionCoordinator::AnalyzeWorldState()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Reset counters
    CurrentMetrics.TotalActors = 0;
    CurrentMetrics.DinosaurCount = 0;
    CurrentMetrics.CharacterCount = 0;
    CurrentMetrics.TerrainActors = 0;
    
    // Count all actors in the world
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor) continue;
        
        CurrentMetrics.TotalActors++;
        
        // Count characters
        if (Cast<ACharacter>(Actor))
        {
            CurrentMetrics.CharacterCount++;
        }
        
        // Count dinosaurs (by name pattern)
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
            ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
            ActorName.Contains(TEXT("ankylo")) || ActorName.Contains(TEXT("parasauro")))
        {
            CurrentMetrics.DinosaurCount++;
        }
        
        // Count terrain actors
        if (Cast<ALandscape>(Actor) || ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
        {
            CurrentMetrics.TerrainActors++;
        }
    }
}

void UProductionCoordinator::CoordinateAgentPriorities()
{
    // Adjust agent priorities based on current production needs
    for (FDir_AgentTask& Task : AgentTasks)
    {
        // Boost priority for blocked agents
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            Task.Priority = FMath::Min(Task.Priority + 0.1f, 10.0f);
        }
        
        // Reduce priority for completed tasks
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            Task.Priority = FMath::Max(Task.Priority - 0.05f, 0.1f);
        }
        
        // Increase priority for overdue tasks
        if (FDateTime::Now() > Task.EstimatedCompletion && Task.Status == EDir_AgentStatus::Working)
        {
            Task.Priority = FMath::Min(Task.Priority + 0.2f, 10.0f);
        }
    }
}