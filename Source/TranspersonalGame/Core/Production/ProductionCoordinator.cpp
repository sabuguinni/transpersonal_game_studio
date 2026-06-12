#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Landscape.h"
#include "TranspersonalGame/Characters/TranspersonalCharacter.h"
#include "TranspersonalGame/Core/TranspersonalGameMode.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize production state
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    CurrentCycle = 0;
    
    // Initialize default metrics
    CurrentMetrics = FDir_ProductionMetrics();
    
    // Set up initial agent tasks for Cycle 002
    AgentTasks.Empty();
    CompletedMilestones.Empty();
    BlockingIssues.Empty();
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Starting Cycle 002 coordination"));
    
    // Initialize production cycle
    InitializeProductionCycle(2);
    
    // Calculate initial metrics
    CurrentMetrics = CalculateCurrentMetrics();
    
    // Report initial state
    ReportProductionStatus();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update metrics every 5 seconds
    static float MetricsUpdateTimer = 0.0f;
    MetricsUpdateTimer += DeltaTime;
    
    if (MetricsUpdateTimer >= 5.0f)
    {
        CurrentMetrics = CalculateCurrentMetrics();
        UpdateProductionPhase();
        MetricsUpdateTimer = 0.0f;
    }
}

void AProductionCoordinator::InitializeProductionCycle(int32 CycleNumber)
{
    CurrentCycle = CycleNumber;
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing Cycle %d"), CycleNumber);
    
    // Clear previous cycle data
    AgentTasks.Empty();
    BlockingIssues.Empty();
    
    // Set up Cycle 002 agent tasks based on current priorities
    AssignTaskToAgent(TEXT("Engine Architect"), TEXT("Validate core systems architecture and fix compilation issues"), 10.0f);
    AssignTaskToAgent(TEXT("Core Systems Programmer"), TEXT("Implement physics and collision systems for character movement"), 9.0f);
    AssignTaskToAgent(TEXT("Procedural World Generator"), TEXT("Create realistic terrain with height variation and biomes"), 8.0f);
    AssignTaskToAgent(TEXT("Environment Artist"), TEXT("Populate world with vegetation, rocks, and environmental details"), 7.0f);
    AssignTaskToAgent(TEXT("Character Artist"), TEXT("Enhance TranspersonalCharacter with proper animations and stats HUD"), 8.0f);
    AssignTaskToAgent(TEXT("Animation Agent"), TEXT("Implement character movement animations and dinosaur basic behaviors"), 7.0f);
    AssignTaskToAgent(TEXT("Combat & Enemy AI"), TEXT("Create basic dinosaur AI with territorial behavior and threat response"), 6.0f);
    AssignTaskToAgent(TEXT("Lighting & Atmosphere"), TEXT("Set up dynamic day/night cycle and atmospheric effects"), 5.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Assigned %d tasks for Cycle %d"), AgentTasks.Num(), CycleNumber);
}

void AProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.Priority = Priority;
    NewTask.CycleAssigned = CurrentCycle;
    NewTask.CycleCompleted = 0;
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Assigned task to %s - %s (Priority: %.1f)"), 
           *AgentName, *TaskDescription, Priority);
}

void AProductionCoordinator::CompleteAgentTask(const FString& AgentName, const TArray<FString>& Deliverables)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName && Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Completed;
            Task.CycleCompleted = CurrentCycle;
            Task.Deliverables = Deliverables;
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: %s completed task with %d deliverables"), 
                   *AgentName, Deliverables.Num());
            break;
        }
    }
}

void AProductionCoordinator::BlockAgentTask(const FString& AgentName, const FString& BlockingReason)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName && Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Blocked;
            BlockingIssues.AddUnique(FString::Printf(TEXT("%s: %s"), *AgentName, *BlockingReason));
            
            UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: %s blocked - %s"), *AgentName, *BlockingReason);
            break;
        }
    }
}

FDir_ProductionMetrics AProductionCoordinator::CalculateCurrentMetrics()
{
    FDir_ProductionMetrics Metrics;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return Metrics;
    }
    
    // Count total actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    Metrics.TotalActorsInLevel = AllActors.Num();
    
    // Count playable characters
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ATranspersonalCharacter::StaticClass(), Characters);
    Metrics.PlayableCharacters = Characters.Num();
    
    // Count dinosaur actors (placeholder for now - will be updated when dinosaur classes exist)
    Metrics.DinosaurActors = 0;
    
    // Count environment actors (excluding characters and lights)
    Metrics.EnvironmentActors = AllActors.Num() - Characters.Num();
    
    // Check for landscape
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Landscapes);
    Metrics.TerrainCoverage = Landscapes.Num() > 0 ? 100.0f : 0.0f;
    
    // Check game systems
    AGameModeBase* GameMode = UGameplayStatics::GetGameMode(World);
    Metrics.bHasWorkingGameMode = (GameMode != nullptr);
    
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    Metrics.bHasPlayerController = (PC != nullptr);
    
    // Check lighting
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    Metrics.bHasLighting = DirectionalLights.Num() > 0;
    
    // Calculate overall completion percentage
    float CompletionFactors = 0.0f;
    float MaxFactors = 7.0f;
    
    if (Metrics.TotalActorsInLevel > 10) CompletionFactors += 1.0f;
    if (Metrics.PlayableCharacters > 0) CompletionFactors += 1.0f;
    if (Metrics.TerrainCoverage > 0) CompletionFactors += 1.0f;
    if (Metrics.bHasWorkingGameMode) CompletionFactors += 1.0f;
    if (Metrics.bHasPlayerController) CompletionFactors += 1.0f;
    if (Metrics.bHasLighting) CompletionFactors += 1.0f;
    if (Metrics.EnvironmentActors > 5) CompletionFactors += 1.0f;
    
    Metrics.OverallCompletionPercentage = (CompletionFactors / MaxFactors) * 100.0f;
    
    return Metrics;
}

float AProductionCoordinator::GetOverallProgress() const
{
    return CurrentMetrics.OverallCompletionPercentage;
}

TArray<FString> AProductionCoordinator::GetActiveAgents() const
{
    TArray<FString> ActiveAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            ActiveAgents.AddUnique(Task.AgentName);
        }
    }
    
    return ActiveAgents;
}

TArray<FString> AProductionCoordinator::GetBlockedAgents() const
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.AddUnique(Task.AgentName);
        }
    }
    
    return BlockedAgents;
}

bool AProductionCoordinator::ValidateMinimumPlayableState() const
{
    return CurrentMetrics.PlayableCharacters > 0 && 
           CurrentMetrics.bHasWorkingGameMode && 
           CurrentMetrics.bHasPlayerController && 
           CurrentMetrics.TerrainCoverage > 0;
}

bool AProductionCoordinator::CheckMilestone_WalkAround() const
{
    return CurrentMetrics.PlayableCharacters > 0 && 
           CurrentMetrics.TerrainCoverage > 0 && 
           CurrentMetrics.bHasPlayerController;
}

bool AProductionCoordinator::CheckMilestone_BasicSurvival() const
{
    // Will be implemented when survival systems are added
    return false;
}

bool AProductionCoordinator::CheckMilestone_DinosaurEncounters() const
{
    return CurrentMetrics.DinosaurActors > 0 && CheckMilestone_WalkAround();
}

void AProductionCoordinator::CoordinateAgentSequence()
{
    // Sort agents by priority
    AgentTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent sequence coordinated by priority"));
}

FString AProductionCoordinator::GetNextPriorityAgent() const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            return Task.AgentName;
        }
    }
    
    return TEXT("No active agents");
}

void AProductionCoordinator::ReportProductionStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS CYCLE %d ==="), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallCompletionPercentage);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Playable Characters: %d"), CurrentMetrics.PlayableCharacters);
    UE_LOG(LogTemp, Warning, TEXT("Has Game Mode: %s"), CurrentMetrics.bHasWorkingGameMode ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Has Terrain: %s"), CurrentMetrics.TerrainCoverage > 0 ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Minimum Playable: %s"), ValidateMinimumPlayableState() ? TEXT("YES") : TEXT("NO"));
    
    // Report agent status
    int32 ActiveCount = 0, CompletedCount = 0, BlockedCount = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Working: ActiveCount++; break;
            case EDir_AgentStatus::Completed: CompletedCount++; break;
            case EDir_AgentStatus::Blocked: BlockedCount++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Status - Active: %d, Completed: %d, Blocked: %d"), 
           ActiveCount, CompletedCount, BlockedCount);
    
    if (BlockingIssues.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("BLOCKING ISSUES:"));
        for (const FString& Issue : BlockingIssues)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Issue);
        }
    }
}

void AProductionCoordinator::DebugPrintProductionState()
{
    ReportProductionStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("=== DETAILED AGENT TASKS ==="));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        UE_LOG(LogTemp, Log, TEXT("Agent: %s | Status: %d | Priority: %.1f | Task: %s"), 
               *Task.AgentName, (int32)Task.Status, Task.Priority, *Task.TaskDescription);
    }
}

void AProductionCoordinator::ForceRecalculateMetrics()
{
    CurrentMetrics = CalculateCurrentMetrics();
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Metrics recalculated - Progress: %.1f%%"), 
           CurrentMetrics.OverallCompletionPercentage);
}

void AProductionCoordinator::UpdateProductionPhase()
{
    float Progress = CurrentMetrics.OverallCompletionPercentage;
    
    if (Progress < 20.0f)
        CurrentPhase = EDir_ProductionPhase::PreProduction;
    else if (Progress < 40.0f)
        CurrentPhase = EDir_ProductionPhase::CoreSystems;
    else if (Progress < 60.0f)
        CurrentPhase = EDir_ProductionPhase::ContentCreation;
    else if (Progress < 80.0f)
        CurrentPhase = EDir_ProductionPhase::Integration;
    else if (Progress < 95.0f)
        CurrentPhase = EDir_ProductionPhase::Testing;
    else
        CurrentPhase = EDir_ProductionPhase::Polish;
}

void AProductionCoordinator::ValidateAgentDependencies()
{
    // Implementation for dependency validation
    // This will be expanded as more agents come online
}

int32 AProductionCoordinator::CountActorsByClass(UClass* ActorClass) const
{
    if (!ActorClass)
        return 0;
        
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, FoundActors);
    return FoundActors.Num();
}

bool AProductionCoordinator::HasRequiredGameSystems() const
{
    return CurrentMetrics.bHasWorkingGameMode && 
           CurrentMetrics.bHasPlayerController && 
           CurrentMetrics.PlayableCharacters > 0;
}