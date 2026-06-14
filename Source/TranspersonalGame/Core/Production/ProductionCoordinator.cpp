#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    CurrentPhase = EDir_ProductionPhase::Prototype;
    CycleCompletionThreshold = 75.0f;
    CurrentCycleNumber = 20;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgentTasks();
    AnalyzeProductionState();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Studio Director coordination system initialized"));
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic production state analysis
    static float AnalysisTimer = 0.0f;
    AnalysisTimer += DeltaTime;
    
    if (AnalysisTimer >= 30.0f) // Analyze every 30 seconds
    {
        AnalyzeProductionState();
        CheckPhaseTransition();
        AnalysisTimer = 0.0f;
    }
}

void UDir_ProductionCoordinator::InitializeAgentTasks()
{
    AgentTasks.Empty();
    
    // Initialize all 19 agent tasks
    TArray<FString> AgentNames = {
        TEXT("Engine Architect"),
        TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"),
        TEXT("Procedural World Generator"),
        TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"),
        TEXT("Lighting & Atmosphere Agent"),
        TEXT("Character Artist Agent"),
        TEXT("Animation Agent"),
        TEXT("NPC Behavior Agent"),
        TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"),
        TEXT("Quest & Mission Designer"),
        TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"),
        TEXT("VFX Agent"),
        TEXT("QA & Testing Agent"),
        TEXT("Integration & Build Agent")
    };
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentNames[i];
        NewTask.TaskDescription = FString::Printf(TEXT("Cycle %d production tasks"), CurrentCycleNumber);
        NewTask.Status = EDir_AgentStatus::Pending;
        NewTask.Priority = i + 1; // Sequential priority based on chain order
        NewTask.CompletionPercentage = 0.0f;
        NewTask.LastUpdate = FDateTime::Now();
        
        AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d agent tasks"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::UpdateAgentTask(const FString& AgentName, EDir_AgentStatus NewStatus, float CompletionPercentage)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            Task.CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
            Task.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Updated %s - Status: %d, Completion: %.1f%%"), 
                   *AgentName, (int32)NewStatus, CompletionPercentage);
            break;
        }
    }
}

void UDir_ProductionCoordinator::AnalyzeProductionState()
{
    if (!GetWorld())
    {
        return;
    }
    
    CountActorsByCategory();
    UpdatePrototypeCompletion();
    
    CurrentMetrics.LastAnalysis = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Analysis Complete - Total Actors: %d, Prototype: %.1f%%"), 
           CurrentMetrics.TotalActors, CurrentMetrics.PrototypeCompletion);
}

void UDir_ProductionCoordinator::CountActorsByCategory()
{
    if (!GetWorld())
    {
        return;
    }
    
    CurrentMetrics.TotalActors = 0;
    CurrentMetrics.TerrainActors = 0;
    CurrentMetrics.CharacterActors = 0;
    CurrentMetrics.DinosaurActors = 0;
    CurrentMetrics.EnvironmentActors = 0;
    CurrentMetrics.LightingActors = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        CurrentMetrics.TotalActors++;
        
        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Categorize actors
        if (ActorName.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("terrain")))
        {
            CurrentMetrics.TerrainActors++;
        }
        else if (ActorName.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("player")))
        {
            CurrentMetrics.CharacterActors++;
        }
        else if (ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("veloci")) || 
                 ActorLabel.Contains(TEXT("tricera")) || ActorLabel.Contains(TEXT("brachi")))
        {
            CurrentMetrics.DinosaurActors++;
        }
        else if (ActorLabel.Contains(TEXT("tree")) || ActorLabel.Contains(TEXT("rock")) || 
                 ActorLabel.Contains(TEXT("bush")) || ActorLabel.Contains(TEXT("grass")))
        {
            CurrentMetrics.EnvironmentActors++;
        }
        else if (ActorName.Contains(TEXT("light")) || ActorName.Contains(TEXT("sun")) || 
                 ActorName.Contains(TEXT("sky")) || ActorName.Contains(TEXT("fog")))
        {
            CurrentMetrics.LightingActors++;
        }
    }
}

void UDir_ProductionCoordinator::UpdatePrototypeCompletion()
{
    float Completion = 0.0f;
    
    // Prototype completion criteria
    bool bHasPlayerCharacter = CurrentMetrics.CharacterActors > 0;
    bool bHasTerrain = CurrentMetrics.TerrainActors > 0;
    bool bHasDinosaurs = CurrentMetrics.DinosaurActors > 0;
    bool bHasEnvironment = CurrentMetrics.EnvironmentActors > 10;
    bool bHasLighting = CurrentMetrics.LightingActors > 0;
    
    // Each component contributes 20% to completion
    if (bHasPlayerCharacter) Completion += 20.0f;
    if (bHasTerrain) Completion += 20.0f;
    if (bHasDinosaurs) Completion += 20.0f;
    if (bHasEnvironment) Completion += 20.0f;
    if (bHasLighting) Completion += 20.0f;
    
    CurrentMetrics.PrototypeCompletion = Completion;
}

void UDir_ProductionCoordinator::CheckPhaseTransition()
{
    if (CurrentPhase == EDir_ProductionPhase::Prototype && CurrentMetrics.PrototypeCompletion >= CycleCompletionThreshold)
    {
        AdvanceProductionPhase();
    }
}

void UDir_ProductionCoordinator::AdvanceProductionPhase()
{
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
            CurrentPhase = EDir_ProductionPhase::Gold;
            break;
        case EDir_ProductionPhase::Gold:
            // Already at final phase
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Advanced to phase: %d"), (int32)CurrentPhase);
}

bool UDir_ProductionCoordinator::IsPrototypeComplete() const
{
    return CurrentMetrics.PrototypeCompletion >= CycleCompletionThreshold;
}

void UDir_ProductionCoordinator::ForceProductionAnalysis()
{
    AnalyzeProductionState();
    LogProductionStatus();
}

void UDir_ProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Cycle Number: %d"), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Terrain Actors: %d"), CurrentMetrics.TerrainActors);
    UE_LOG(LogTemp, Warning, TEXT("Character Actors: %d"), CurrentMetrics.CharacterActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), CurrentMetrics.DinosaurActors);
    UE_LOG(LogTemp, Warning, TEXT("Environment Actors: %d"), CurrentMetrics.EnvironmentActors);
    UE_LOG(LogTemp, Warning, TEXT("Lighting Actors: %d"), CurrentMetrics.LightingActors);
    UE_LOG(LogTemp, Warning, TEXT("Prototype Completion: %.1f%%"), CurrentMetrics.PrototypeCompletion);
    UE_LOG(LogTemp, Warning, TEXT("================================"));
    
    // Log agent task status
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent: %s | Status: %d | Completion: %.1f%%"), 
               *Task.AgentName, (int32)Task.Status, Task.CompletionPercentage);
    }
}