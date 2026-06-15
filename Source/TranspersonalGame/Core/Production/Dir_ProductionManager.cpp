#include "Dir_ProductionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

void UDir_ProductionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Production Manager initialized - coordinating 19 AI agents"));
    
    InitializeAgents();
    CurrentMetrics = FDir_ProductionMetrics();
    CurrentMetrics.CurrentPhase = EDir_ProductionPhase::Prototype;
    
    // Start with Milestone 1: Walk Around prototype
    UpdateProductionMetrics();
}

void UDir_ProductionManager::Deinitialize()
{
    AgentTasks.Empty();
    Super::Deinitialize();
}

void UDir_ProductionManager::InitializeAgents()
{
    // Register all 19 AI agents with their specializations
    RegisterAgent(1, TEXT("Studio Director"));
    RegisterAgent(2, TEXT("Engine Architect"));
    RegisterAgent(3, TEXT("Core Systems Programmer"));
    RegisterAgent(4, TEXT("Performance Optimizer"));
    RegisterAgent(5, TEXT("Procedural World Generator"));
    RegisterAgent(6, TEXT("Environment Artist"));
    RegisterAgent(7, TEXT("Architecture & Interior Agent"));
    RegisterAgent(8, TEXT("Lighting & Atmosphere Agent"));
    RegisterAgent(9, TEXT("Character Artist Agent"));
    RegisterAgent(10, TEXT("Animation Agent"));
    RegisterAgent(11, TEXT("NPC Behavior Agent"));
    RegisterAgent(12, TEXT("Combat & Enemy AI Agent"));
    RegisterAgent(13, TEXT("Crowd & Traffic Simulation"));
    RegisterAgent(14, TEXT("Quest & Mission Designer"));
    RegisterAgent(15, TEXT("Narrative & Dialogue Agent"));
    RegisterAgent(16, TEXT("Audio Agent"));
    RegisterAgent(17, TEXT("VFX Agent"));
    RegisterAgent(18, TEXT("QA & Testing Agent"));
    RegisterAgent(19, TEXT("Integration & Build Agent"));
}

void UDir_ProductionManager::RegisterAgent(int32 AgentID, const FString& AgentName)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.AgentName = AgentName;
    NewTask.CurrentTask = TEXT("Initializing...");
    NewTask.Status = EDir_AgentStatus::Idle;
    NewTask.CompletionPercentage = 0.0f;
    NewTask.LastUpdate = FDateTime::Now();
    
    AgentTasks.Add(AgentID, NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("Registered Agent #%d: %s"), AgentID, *AgentName);
}

void UDir_ProductionManager::UpdateAgentTask(int32 AgentID, const FString& TaskDescription, EDir_AgentStatus Status, float Completion)
{
    if (FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        Task->CurrentTask = TaskDescription;
        Task->Status = Status;
        Task->CompletionPercentage = FMath::Clamp(Completion, 0.0f, 100.0f);
        Task->LastUpdate = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s [%.1f%%]"), 
               AgentID, *Task->AgentName, *TaskDescription, Completion);
    }
}

void UDir_ProductionManager::IncrementAgentFiles(int32 AgentID, int32 FileCount)
{
    if (FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        Task->FilesCreated += FileCount;
        Task->LastUpdate = FDateTime::Now();
    }
}

void UDir_ProductionManager::IncrementAgentCommands(int32 AgentID, int32 CommandCount)
{
    if (FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        Task->UE5CommandsExecuted += CommandCount;
        Task->LastUpdate = FDateTime::Now();
    }
}

TArray<FDir_AgentTask> UDir_ProductionManager::GetAllAgentTasks() const
{
    TArray<FDir_AgentTask> Tasks;
    for (const auto& Pair : AgentTasks)
    {
        Tasks.Add(Pair.Value);
    }
    return Tasks;
}

FDir_AgentTask UDir_ProductionManager::GetAgentTask(int32 AgentID) const
{
    if (const FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        return *Task;
    }
    return FDir_AgentTask();
}

void UDir_ProductionManager::UpdateProductionMetrics()
{
    CurrentMetrics.TotalActors = CountActorsInLevel();
    CurrentMetrics.DinosaurCount = CountDinosaursInLevel();
    CurrentMetrics.LastBuild = FDateTime::Now();
    
    CalculateOverallProgress();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Metrics - Actors: %d, Dinosaurs: %d, Progress: %.1f%%"),
           CurrentMetrics.TotalActors, CurrentMetrics.DinosaurCount, CurrentMetrics.OverallProgress);
}

FDir_ProductionMetrics UDir_ProductionManager::GetProductionMetrics() const
{
    return CurrentMetrics;
}

void UDir_ProductionManager::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentMetrics.CurrentPhase = NewPhase;
    
    FString PhaseNames[] = {
        TEXT("PreProduction"), TEXT("Prototype"), TEXT("VerticalSlice"), 
        TEXT("Alpha"), TEXT("Beta"), TEXT("Gold")
    };
    
    UE_LOG(LogTemp, Warning, TEXT("Production Phase changed to: %s"), 
           *PhaseNames[static_cast<int32>(NewPhase)]);
}

bool UDir_ProductionManager::CheckMilestone1Complete() const
{
    // Milestone 1: Walk Around
    // - Character can move with WASD
    // - Landscape with terrain
    // - 3-5 dinosaur meshes placed
    // - Basic lighting
    
    bool HasCharacter = false;
    bool HasTerrain = false;
    bool HasDinosaurs = CurrentMetrics.DinosaurCount >= 3;
    bool HasLighting = false;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            FString ActorName = Actor->GetName().ToLower();
            
            if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("pawn")))
            {
                HasCharacter = true;
            }
            else if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
            {
                HasTerrain = true;
            }
            else if (ActorName.Contains(TEXT("directionallight")) || ActorName.Contains(TEXT("skylight")))
            {
                HasLighting = true;
            }
        }
    }
    
    return HasCharacter && HasTerrain && HasDinosaurs && HasLighting;
}

bool UDir_ProductionManager::CheckMilestone2Complete() const
{
    // Milestone 2: Basic Survival
    // - Health/hunger/thirst systems
    // - Basic crafting
    // - Day/night cycle
    
    return CurrentMetrics.CompletedSystems >= 5;
}

bool UDir_ProductionManager::CheckMilestone3Complete() const
{
    // Milestone 3: Dinosaur Interaction
    // - Dinosaur AI behaviors
    // - Combat system
    // - Taming mechanics
    
    return CurrentMetrics.CompletedSystems >= 10 && CurrentMetrics.DinosaurCount >= 10;
}

void UDir_ProductionManager::LogPerformanceMetrics()
{
    FString Report = FString::Printf(
        TEXT("=== PRODUCTION PERFORMANCE REPORT ===\n")
        TEXT("Total Actors: %d / 8000 limit\n")
        TEXT("Dinosaurs: %d / 150 limit\n")
        TEXT("Active Agents: %d / 19\n")
        TEXT("Overall Progress: %.1f%%\n")
        TEXT("Current Phase: %d\n"),
        CurrentMetrics.TotalActors, CurrentMetrics.DinosaurCount,
        AgentTasks.Num(), CurrentMetrics.OverallProgress,
        static_cast<int32>(CurrentMetrics.CurrentPhase)
    );
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

bool UDir_ProductionManager::IsWithinActorLimits() const
{
    return CurrentMetrics.TotalActors <= 8000 && CurrentMetrics.DinosaurCount <= 150;
}

void UDir_ProductionManager::EnforceActorLimits()
{
    if (!IsWithinActorLimits())
    {
        UE_LOG(LogTemp, Error, TEXT("Actor limits exceeded! Actors: %d, Dinosaurs: %d"), 
               CurrentMetrics.TotalActors, CurrentMetrics.DinosaurCount);
        
        // This would trigger the CAP enforcement script
        // Implementation delegated to UE5 Python commands
    }
}

void UDir_ProductionManager::CalculateOverallProgress()
{
    float TotalProgress = 0.0f;
    int32 ActiveAgents = 0;
    
    for (const auto& Pair : AgentTasks)
    {
        const FDir_AgentTask& Task = Pair.Value;
        if (Task.Status != EDir_AgentStatus::Idle)
        {
            TotalProgress += Task.CompletionPercentage;
            ActiveAgents++;
        }
    }
    
    CurrentMetrics.OverallProgress = ActiveAgents > 0 ? TotalProgress / ActiveAgents : 0.0f;
    CurrentMetrics.CompletedSystems = 0;
    
    // Count completed systems based on agent completion
    for (const auto& Pair : AgentTasks)
    {
        if (Pair.Value.CompletionPercentage >= 90.0f)
        {
            CurrentMetrics.CompletedSystems++;
        }
    }
}

int32 UDir_ProductionManager::CountActorsInLevel() const
{
    int32 Count = 0;
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            Count++;
        }
    }
    return Count;
}

int32 UDir_ProductionManager::CountDinosaursInLevel() const
{
    int32 Count = 0;
    if (UWorld* World = GetWorld())
    {
        TArray<FString> DinoLabels = {
            TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"),
            TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")
        };
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            FString ActorLabel = ActorItr->GetActorLabel().ToLower();
            for (const FString& DinoLabel : DinoLabels)
            {
                if (ActorLabel.Contains(DinoLabel))
                {
                    Count++;
                    break;
                }
            }
        }
    }
    return Count;
}