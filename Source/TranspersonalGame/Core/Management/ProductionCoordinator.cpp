#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f; // Update every 5 seconds
    
    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionTasks();
    UpdateProductionMetrics();
    bInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator initialized - Cycle %d"), CurrentCycle);
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bInitialized)
        return;
    
    LastMetricsUpdate += DeltaTime;
    
    // Update metrics every 10 seconds
    if (LastMetricsUpdate >= 10.0f)
    {
        UpdateProductionMetrics();
        ValidateGameState();
        LastMetricsUpdate = 0.0f;
    }
}

void AProductionCoordinator::InitializeProductionTasks()
{
    AgentTasks.Empty();
    
    // Milestone 1 tasks for each agent
    AssignTaskToAgent(TEXT("Agent02_EngineArchitect"), TEXT("Define core architecture and compilation rules"), 10.0f);
    AssignTaskToAgent(TEXT("Agent03_CoreSystems"), TEXT("Implement physics and collision systems"), 9.0f);
    AssignTaskToAgent(TEXT("Agent05_WorldGenerator"), TEXT("Create terrain with hills and valleys"), 8.0f);
    AssignTaskToAgent(TEXT("Agent06_Environment"), TEXT("Place trees, rocks, and vegetation"), 7.0f);
    AssignTaskToAgent(TEXT("Agent08_Lighting"), TEXT("Set up day/night cycle and atmosphere"), 6.0f);
    AssignTaskToAgent(TEXT("Agent09_Character"), TEXT("Create playable character with movement"), 9.0f);
    AssignTaskToAgent(TEXT("Agent10_Animation"), TEXT("Add character animations and IK"), 5.0f);
    AssignTaskToAgent(TEXT("Agent11_NPCBehavior"), TEXT("Create basic dinosaur AI"), 4.0f);
    AssignTaskToAgent(TEXT("Agent12_Combat"), TEXT("Implement survival HUD and basic combat"), 8.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d production tasks"), AgentTasks.Num());
}

void AProductionCoordinator::UpdateProductionMetrics()
{
    if (!GetWorld())
        return;
    
    // Reset metrics
    CurrentMetrics = FDir_ProductionMetrics();
    
    // Count all actors
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || Actor == this)
            continue;
        
        CurrentMetrics.TotalActors++;
        
        // Categorize actors
        if (Actor->IsA<ACharacter>())
        {
            CurrentMetrics.CharacterActors++;
        }
        else if (Actor->GetName().Contains(TEXT("Dinosaur")) || 
                 Actor->GetName().Contains(TEXT("TRex")) ||
                 Actor->GetName().Contains(TEXT("Raptor")))
        {
            CurrentMetrics.DinosaurActors++;
        }
        else if (Actor->IsA<ALandscape>())
        {
            CurrentMetrics.TerrainActors++;
        }
        else if (Actor->IsA<ADirectionalLight>() || Actor->IsA<ASkyLight>())
        {
            CurrentMetrics.LightingActors++;
        }
    }
    
    // Calculate overall progress
    CurrentMetrics.ProductionProgress = CalculateOverallProgress();
    
    UE_LOG(LogTemp, Warning, TEXT("Metrics Updated - Total: %d, Characters: %d, Dinosaurs: %d, Progress: %.1f%%"), 
           CurrentMetrics.TotalActors, CurrentMetrics.CharacterActors, 
           CurrentMetrics.DinosaurActors, CurrentMetrics.ProductionProgress * 100.0f);
}

void AProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.bCompleted = false;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Task assigned to %s: %s"), *AgentName, *TaskDescription);
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

float AProductionCoordinator::CalculateOverallProgress() const
{
    if (AgentTasks.Num() == 0)
        return 0.0f;
    
    int32 CompletedTasks = 0;
    float WeightedProgress = 0.0f;
    float TotalWeight = 0.0f;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TotalWeight += Task.Priority;
        if (Task.bCompleted)
        {
            CompletedTasks++;
            WeightedProgress += Task.Priority;
        }
    }
    
    return TotalWeight > 0.0f ? WeightedProgress / TotalWeight : 0.0f;
}

TArray<FString> AProductionCoordinator::GetPendingTasks() const
{
    TArray<FString> PendingTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bCompleted)
        {
            PendingTasks.Add(FString::Printf(TEXT("%s: %s"), *Task.AgentName, *Task.TaskDescription));
        }
    }
    
    return PendingTasks;
}

bool AProductionCoordinator::IsPhaseComplete() const
{
    if (CurrentPhase == TEXT("Milestone1_WalkAround"))
    {
        return HasPlayableCharacter() && 
               HasTerrainWithVariation() && 
               HasDinosaurActors() && 
               HasBasicLighting();
    }
    
    return false;
}

bool AProductionCoordinator::HasPlayableCharacter() const
{
    return CurrentMetrics.CharacterActors > 0;
}

bool AProductionCoordinator::HasTerrainWithVariation() const
{
    return CurrentMetrics.TerrainActors > 0;
}

bool AProductionCoordinator::HasDinosaurActors() const
{
    return CurrentMetrics.DinosaurActors >= 3; // Need at least 3 dinosaurs
}

bool AProductionCoordinator::HasBasicLighting() const
{
    return CurrentMetrics.LightingActors > 0;
}

void AProductionCoordinator::ValidateGameState()
{
    // Log current milestone progress
    bool bCharacterReady = HasPlayableCharacter();
    bool bTerrainReady = HasTerrainWithVariation();
    bool bDinosaursReady = HasDinosaurActors();
    bool bLightingReady = HasBasicLighting();
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Status - Character: %s, Terrain: %s, Dinosaurs: %s, Lighting: %s"),
           bCharacterReady ? TEXT("READY") : TEXT("PENDING"),
           bTerrainReady ? TEXT("READY") : TEXT("PENDING"),
           bDinosaursReady ? TEXT("READY") : TEXT("PENDING"),
           bLightingReady ? TEXT("READY") : TEXT("PENDING"));
    
    if (IsPhaseComplete())
    {
        UE_LOG(LogTemp, Error, TEXT("MILESTONE 1 COMPLETE! Moving to next phase..."));
    }
}