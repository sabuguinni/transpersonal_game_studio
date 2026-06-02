#include "Dir_ProductionMetrics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"

void UDir_ProductionMetrics::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Metrics initialized"));
    
    InitializeAgentTasks();
    
    // Start metrics update timer (every 5 seconds)
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MetricsUpdateTimer, this, &UDir_ProductionMetrics::UpdateMetricsTimer, 5.0f, true);
    }
}

void UDir_ProductionMetrics::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    Super::Deinitialize();
}

void UDir_ProductionMetrics::InitializeAgentTasks()
{
    // Initialize all 19 agents
    TArray<TPair<int32, FString>> AgentDefinitions = {
        {1, TEXT("Studio Director")},
        {2, TEXT("Engine Architect")},
        {3, TEXT("Core Systems Programmer")},
        {4, TEXT("Performance Optimizer")},
        {5, TEXT("Procedural World Generator")},
        {6, TEXT("Environment Artist")},
        {7, TEXT("Architecture & Interior Agent")},
        {8, TEXT("Lighting & Atmosphere Agent")},
        {9, TEXT("Character Artist Agent")},
        {10, TEXT("Animation Agent")},
        {11, TEXT("NPC Behavior Agent")},
        {12, TEXT("Combat & Enemy AI Agent")},
        {13, TEXT("Crowd & Traffic Simulation")},
        {14, TEXT("Quest & Mission Designer")},
        {15, TEXT("Narrative & Dialogue Agent")},
        {16, TEXT("Audio Agent")},
        {17, TEXT("VFX Agent")},
        {18, TEXT("QA & Testing Agent")},
        {19, TEXT("Integration & Build Agent")}
    };

    for (const auto& AgentDef : AgentDefinitions)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentNumber = AgentDef.Key;
        NewTask.AgentName = AgentDef.Value;
        NewTask.CurrentTask = TEXT("Initializing...");
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.ProgressPercent = 0.0f;
        
        AgentTasks.Add(AgentDef.Key, NewTask);
    }
}

void UDir_ProductionMetrics::UpdateProductionStats()
{
    if (!GetWorld())
    {
        return;
    }

    CurrentStats.TotalActors = 0;
    CurrentStats.DinosaurCount = 0;
    CurrentStats.CharacterCount = 0;
    CurrentStats.LandscapeCount = 0;

    // Count all actors in the world
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsPendingKill())
        {
            continue;
        }

        CurrentStats.TotalActors++;

        // Count characters
        if (Actor->IsA<ACharacter>())
        {
            CurrentStats.CharacterCount++;
        }

        // Count landscapes
        if (Actor->IsA<ALandscape>())
        {
            CurrentStats.LandscapeCount++;
        }

        // Count dinosaurs by name patterns
        FString ActorName = Actor->GetName().ToLower();
        if (ActorName.Contains(TEXT("trex")) || 
            ActorName.Contains(TEXT("veloci")) || 
            ActorName.Contains(TEXT("tricera")) || 
            ActorName.Contains(TEXT("brachi")) || 
            ActorName.Contains(TEXT("ankylo")) || 
            ActorName.Contains(TEXT("parasauro")))
        {
            CurrentStats.DinosaurCount++;
        }
    }

    // Get performance metrics
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentStats.CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    }

    // Estimate memory usage (simplified)
    CurrentStats.MemoryUsageGB = CurrentStats.TotalActors * 0.001f; // Rough estimate

    CurrentStats.LastUpdated = FDateTime::Now();

    UE_LOG(LogTemp, Log, TEXT("Production Stats Updated - Actors: %d, Dinos: %d, Characters: %d, Landscapes: %d"), 
           CurrentStats.TotalActors, CurrentStats.DinosaurCount, CurrentStats.CharacterCount, CurrentStats.LandscapeCount);
}

void UDir_ProductionMetrics::UpdateAgentTask(int32 AgentNumber, const FString& TaskDescription, EDir_AgentStatus Status, float Progress)
{
    if (FDir_AgentTask* Task = AgentTasks.Find(AgentNumber))
    {
        Task->CurrentTask = TaskDescription;
        Task->Status = Status;
        Task->ProgressPercent = FMath::Clamp(Progress, 0.0f, 100.0f);
        Task->LastUpdate = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Agent %d (%s) - Task: %s, Status: %d, Progress: %.1f%%"), 
               AgentNumber, *Task->AgentName, *TaskDescription, (int32)Status, Progress);
    }
}

TArray<FDir_AgentTask> UDir_ProductionMetrics::GetAllAgentTasks() const
{
    TArray<FDir_AgentTask> Tasks;
    for (const auto& TaskPair : AgentTasks)
    {
        Tasks.Add(TaskPair.Value);
    }
    return Tasks;
}

FDir_AgentTask UDir_ProductionMetrics::GetAgentTask(int32 AgentNumber) const
{
    if (const FDir_AgentTask* Task = AgentTasks.Find(AgentNumber))
    {
        return *Task;
    }
    return FDir_AgentTask();
}

void UDir_ProductionMetrics::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentStats.CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Production phase changed to: %d"), (int32)NewPhase);
}

bool UDir_ProductionMetrics::IsPhaseComplete(EDir_ProductionPhase Phase) const
{
    switch (Phase)
    {
        case EDir_ProductionPhase::Prototype:
            return IsWalkAroundMilestoneComplete();
        case EDir_ProductionPhase::Production:
            return CurrentStats.DinosaurCount >= 5 && CurrentStats.CharacterCount >= 1 && CurrentStats.LandscapeCount >= 1;
        default:
            return false;
    }
}

TArray<int32> UDir_ProductionMetrics::GetBlockedAgents() const
{
    TArray<int32> BlockedAgents;
    for (const auto& TaskPair : AgentTasks)
    {
        if (TaskPair.Value.Status == EDir_AgentStatus::Blocked || TaskPair.Value.Status == EDir_AgentStatus::Failed)
        {
            BlockedAgents.Add(TaskPair.Key);
        }
    }
    return BlockedAgents;
}

TArray<int32> UDir_ProductionMetrics::GetCriticalPathAgents() const
{
    // Critical path for Walk Around milestone: 2, 3, 5, 9, 12
    TArray<int32> CriticalPath = {2, 3, 5, 9, 12};
    return CriticalPath;
}

bool UDir_ProductionMetrics::IsWalkAroundMilestoneComplete() const
{
    return ValidateMinimumViablePrototype();
}

float UDir_ProductionMetrics::GetOverallProgress() const
{
    float TotalProgress = 0.0f;
    int32 ActiveAgents = 0;
    
    for (const auto& TaskPair : AgentTasks)
    {
        if (TaskPair.Value.Status != EDir_AgentStatus::Idle)
        {
            TotalProgress += TaskPair.Value.ProgressPercent;
            ActiveAgents++;
        }
    }
    
    return ActiveAgents > 0 ? TotalProgress / ActiveAgents : 0.0f;
}

void UDir_ProductionMetrics::UpdateMetricsTimer()
{
    UpdateProductionStats();
}

bool UDir_ProductionMetrics::ValidateMinimumViablePrototype() const
{
    // Walk Around Milestone requirements:
    // ✓ ThirdPersonCharacter (at least 1 character)
    // ✓ Landscape with terrain (at least 1 landscape)
    // ✓ 3-5 dinosaur meshes placed
    // ✓ Basic lighting and atmosphere
    
    bool HasCharacter = CurrentStats.CharacterCount >= 1;
    bool HasLandscape = CurrentStats.LandscapeCount >= 1;
    bool HasDinosaurs = CurrentStats.DinosaurCount >= 3 && CurrentStats.DinosaurCount <= 150; // Respect limits
    bool ReasonableActorCount = CurrentStats.TotalActors > 10 && CurrentStats.TotalActors < 20000; // Not empty, not overloaded
    
    return HasCharacter && HasLandscape && HasDinosaurs && ReasonableActorCount;
}