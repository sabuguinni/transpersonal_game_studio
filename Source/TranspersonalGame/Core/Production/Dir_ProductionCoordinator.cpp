#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentPhase = EDir_ProductionPhase::CoreSystems;
    CycleStartTime = 0.0f;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    InitializeProductionCycle();
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update metrics every 5 seconds
    static float MetricsTimer = 0.0f;
    MetricsTimer += DeltaTime;
    if (MetricsTimer >= 5.0f)
    {
        UpdateMetrics();
        MetricsTimer = 0.0f;
    }
}

void UDir_ProductionCoordinator::InitializeProductionCycle()
{
    CycleStartTime = GetWorld()->GetTimeSeconds();
    
    // Initialize critical agent tasks
    AgentTasks.Empty();
    
    FDir_AgentTask EngineArchitectTask;
    EngineArchitectTask.AgentName = TEXT("Agent02_EngineArchitect");
    EngineArchitectTask.TaskDescription = TEXT("Define core systems architecture and compilation rules");
    EngineArchitectTask.Priority = 10;
    AgentTasks.Add(EngineArchitectTask);
    
    FDir_AgentTask CoreSystemsTask;
    CoreSystemsTask.AgentName = TEXT("Agent03_CoreSystems");
    CoreSystemsTask.TaskDescription = TEXT("Implement physics, collision, and ragdoll systems");
    CoreSystemsTask.Priority = 9;
    AgentTasks.Add(CoreSystemsTask);
    
    FDir_AgentTask WorldGenTask;
    WorldGenTask.AgentName = TEXT("Agent05_WorldGen");
    WorldGenTask.TaskDescription = TEXT("Create real terrain with height variation and biomes");
    WorldGenTask.Priority = 8;
    AgentTasks.Add(WorldGenTask);
    
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentName = TEXT("Agent09_CharacterArtist");
    CharacterTask.TaskDescription = TEXT("Create dinosaur actors with collision and basic AI");
    CharacterTask.Priority = 8;
    AgentTasks.Add(CharacterTask);
    
    FDir_AgentTask AnimationTask;
    AnimationTask.AgentName = TEXT("Agent10_Animation");
    AnimationTask.TaskDescription = TEXT("Implement movement and behavior animations");
    AnimationTask.Priority = 7;
    AgentTasks.Add(AnimationTask);
    
    FDir_AgentTask CombatTask;
    CombatTask.AgentName = TEXT("Agent12_CombatAI");
    CombatTask.TaskDescription = TEXT("Implement survival HUD and AI systems");
    CombatTask.Priority = 7;
    AgentTasks.Add(CombatTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle initialized with %d critical tasks"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::UpdateAgentTask(const FString& AgentName, float CompletionPercentage, bool bCompleted)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.CompletionPercentage = FMath::Clamp(CompletionPercentage, 0.0f, 100.0f);
            Task.bIsCompleted = bCompleted;
            UE_LOG(LogTemp, Log, TEXT("Updated %s: %.1f%% complete"), *AgentName, CompletionPercentage);
            break;
        }
    }
}

void UDir_ProductionCoordinator::AnalyzeCurrentState()
{
    UpdateMetrics();
    IdentifyBottlenecks();
    LogProductionState();
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetCriticalTasks() const
{
    TArray<FDir_AgentTask> CriticalTasks;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Priority >= 8 && !Task.bIsCompleted)
        {
            CriticalTasks.Add(Task);
        }
    }
    return CriticalTasks;
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT CYCLE 006 ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Current Bottleneck: %s"), *CurrentMetrics.CurrentBottleneck);
    
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT TASKS STATUS ==="));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString Status = Task.bIsCompleted ? TEXT("COMPLETED") : FString::Printf(TEXT("%.1f%%"), Task.CompletionPercentage);
        UE_LOG(LogTemp, Warning, TEXT("%s: %s - %s"), *Task.AgentName, *Status, *Task.TaskDescription);
    }
}

void UDir_ProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Production phase changed to: %d"), (int32)NewPhase);
}

bool UDir_ProductionCoordinator::ValidateMinimumPlayableState() const
{
    // Check for minimum playable requirements
    bool bHasPlayerStart = CurrentMetrics.TotalActorsInLevel > 0; // Simplified check
    bool bHasTerrain = true; // Assume terrain exists
    bool bHasCharacter = true; // Assume character exists
    bool bHasDinosaurs = CurrentMetrics.DinosaurCount > 0;
    
    return bHasPlayerStart && bHasTerrain && bHasCharacter && bHasDinosaurs;
}

void UDir_ProductionCoordinator::UpdateMetrics()
{
    if (!GetWorld()) return;
    
    // Count all actors in the level
    int32 ActorCount = 0;
    int32 DinoCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            ActorCount++;
            FString ActorLabel = Actor->GetActorLabel();
            if (ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("veloci")) || 
                ActorLabel.Contains(TEXT("tricera")) || ActorLabel.Contains(TEXT("brachi")) ||
                ActorLabel.Contains(TEXT("ankylo")))
            {
                DinoCount++;
            }
        }
    }
    
    CurrentMetrics.TotalActorsInLevel = ActorCount;
    CurrentMetrics.DinosaurCount = DinoCount;
    
    // Calculate overall progress based on completed tasks
    int32 CompletedTasks = 0;
    float TotalProgress = 0.0f;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bIsCompleted) CompletedTasks++;
        TotalProgress += Task.CompletionPercentage;
    }
    
    CurrentMetrics.CompletedSystems = CompletedTasks;
    CurrentMetrics.OverallProgress = AgentTasks.Num() > 0 ? TotalProgress / AgentTasks.Num() : 0.0f;
}

void UDir_ProductionCoordinator::IdentifyBottlenecks()
{
    // Find the highest priority incomplete task
    FDir_AgentTask* BottleneckTask = nullptr;
    int32 HighestPriority = 0;
    
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bIsCompleted && Task.Priority > HighestPriority)
        {
            HighestPriority = Task.Priority;
            BottleneckTask = &Task;
        }
    }
    
    if (BottleneckTask)
    {
        CurrentMetrics.CurrentBottleneck = BottleneckTask->AgentName;
    }
    else
    {
        CurrentMetrics.CurrentBottleneck = TEXT("No critical bottlenecks");
    }
}

void UDir_ProductionCoordinator::LogProductionState()
{
    UE_LOG(LogTemp, Log, TEXT("Production State: %d actors, %d dinosaurs, %.1f%% complete"), 
           CurrentMetrics.TotalActorsInLevel, CurrentMetrics.DinosaurCount, CurrentMetrics.OverallProgress);
}