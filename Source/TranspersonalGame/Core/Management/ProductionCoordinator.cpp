#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize milestone states
    bTerrainComplete = false;
    bCharacterFunctional = false;
    bDinosaursPlaced = false;
    bLightingSetup = false;
    bBasicGameplay = false;
    
    // Update every 5 seconds
    UpdateInterval = 5.0f;
    TimeSinceLastUpdate = 0.0f;
    
    // Initialize default agent tasks
    AgentTasks.Empty();
    
    // Set up initial critical tasks
    AssignTaskToAgent(TEXT("Agent_02_Engine_Architect"), TEXT("Define core gameplay systems architecture"), 10.0f);
    AssignTaskToAgent(TEXT("Agent_03_Core_Systems"), TEXT("Implement physics and collision systems"), 9.0f);
    AssignTaskToAgent(TEXT("Agent_05_World_Generator"), TEXT("Create varied terrain with PCG"), 8.0f);
    AssignTaskToAgent(TEXT("Agent_09_Character_Artist"), TEXT("Enhance character with proper mesh and animations"), 7.0f);
    AssignTaskToAgent(TEXT("Agent_12_Combat_AI"), TEXT("Implement basic dinosaur AI behaviors"), 6.0f);
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initial metrics update
    UpdateProductionMetrics();
    ValidateMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized - tracking %d agent tasks"), AgentTasks.Num());
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdateProductionMetrics();
        ValidateMilestones();
        TimeSinceLastUpdate = 0.0f;
    }
}

void AProductionCoordinator::UpdateProductionMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Reset counters
    CurrentMetrics.TotalActors = 0;
    CurrentMetrics.CharacterActors = 0;
    CurrentMetrics.DinosaurActors = 0;
    CurrentMetrics.TerrainActors = 0;
    
    // Count all actors in the world
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }
        
        CurrentMetrics.TotalActors++;
        
        FString ActorName = Actor->GetName().ToLower();
        
        // Classify actors
        if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
        {
            CurrentMetrics.CharacterActors++;
        }
        else if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("trex")) || 
                 ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachio")))
        {
            CurrentMetrics.DinosaurActors++;
        }
        else if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
        {
            CurrentMetrics.TerrainActors++;
        }
    }
    
    // Calculate overall progress
    CurrentMetrics.OverallProgress = CalculateOverallProgress();
    CurrentMetrics.LastUpdated = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("Production Metrics Updated - Total: %d, Characters: %d, Dinosaurs: %d, Terrain: %d, Progress: %.1f%%"),
           CurrentMetrics.TotalActors, CurrentMetrics.CharacterActors, CurrentMetrics.DinosaurActors, 
           CurrentMetrics.TerrainActors, CurrentMetrics.OverallProgress);
}

void AProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bCompleted = false;
    NewTask.AssignedTime = FDateTime::Now();
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to %s: %s (Priority: %.1f)"), 
           *AgentName, *TaskDescription, Priority);
}

void AProductionCoordinator::CompleteAgentTask(const FString& AgentName)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName && !Task.bCompleted)
        {
            Task.bCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Task completed by %s: %s"), *AgentName, *Task.TaskDescription);
            break;
        }
    }
}

float AProductionCoordinator::CalculateOverallProgress()
{
    // Define milestone weights
    float TerrainWeight = 0.3f;
    float CharacterWeight = 0.25f;
    float DinosaurWeight = 0.25f;
    float LightingWeight = 0.1f;
    float GameplayWeight = 0.1f;
    
    // Calculate individual progress percentages
    float TerrainProgress = bTerrainComplete ? 100.0f : FMath::Min(CurrentMetrics.TerrainActors * 100.0f, 100.0f);
    float CharacterProgress = bCharacterFunctional ? 100.0f : FMath::Min(CurrentMetrics.CharacterActors * 100.0f, 100.0f);
    float DinosaurProgress = bDinosaursPlaced ? 100.0f : FMath::Min((CurrentMetrics.DinosaurActors / 5.0f) * 100.0f, 100.0f);
    float LightingProgress = bLightingSetup ? 100.0f : 0.0f;
    float GameplayProgress = bBasicGameplay ? 100.0f : 0.0f;
    
    // Calculate weighted average
    float WeightedProgress = (TerrainProgress * TerrainWeight) + 
                           (CharacterProgress * CharacterWeight) + 
                           (DinosaurProgress * DinosaurWeight) + 
                           (LightingProgress * LightingWeight) + 
                           (GameplayProgress * GameplayWeight);
    
    return WeightedProgress;
}

TArray<FDir_AgentTask> AProductionCoordinator::GetPendingTasks()
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    // Sort by priority (highest first)
    PendingTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    return PendingTasks;
}

FDir_ProductionMetrics AProductionCoordinator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void AProductionCoordinator::ValidateMilestones()
{
    // Milestone 1 validation criteria
    bTerrainComplete = (CurrentMetrics.TerrainActors >= 1);
    bCharacterFunctional = (CurrentMetrics.CharacterActors >= 1);
    bDinosaursPlaced = (CurrentMetrics.DinosaurActors >= 5);
    
    // Check for lighting setup (basic assumption for now)
    bLightingSetup = (CurrentMetrics.TotalActors > 10); // Rough estimate
    
    // Basic gameplay check (if character and terrain exist)
    bBasicGameplay = (bTerrainComplete && bCharacterFunctional);
    
    if (IsMilestone1Complete())
    {
        UE_LOG(LogTemp, Warning, TEXT("MILESTONE 1 COMPLETE - Minimum Viable Playable Prototype achieved!"));
    }
}

bool AProductionCoordinator::IsMilestone1Complete() const
{
    return bTerrainComplete && bCharacterFunctional && bDinosaursPlaced && bLightingSetup && bBasicGameplay;
}

void AProductionCoordinator::LogProductionState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Character Actors: %d"), CurrentMetrics.CharacterActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), CurrentMetrics.DinosaurActors);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Actors: %d"), CurrentMetrics.TerrainActors);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    
    UE_LOG(LogTemp, Warning, TEXT("=== MILESTONE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Terrain Complete: %s"), bTerrainComplete ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Character Functional: %s"), bCharacterFunctional ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs Placed: %s"), bDinosaursPlaced ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Lighting Setup: %s"), bLightingSetup ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Basic Gameplay: %s"), bBasicGameplay ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== PENDING TASKS ==="));
    TArray<FDir_AgentTask> PendingTasks = GetPendingTasks();
    for (const FDir_AgentTask& Task : PendingTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s (Priority: %.1f)"), *Task.AgentName, *Task.TaskDescription, Task.Priority);
    }
}

void AProductionCoordinator::ResetAllTasks()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.bCompleted = false;
        Task.AssignedTime = FDateTime::Now();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("All agent tasks reset to pending status"));
}