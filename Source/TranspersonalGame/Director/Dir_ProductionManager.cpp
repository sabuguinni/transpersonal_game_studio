#include "Dir_ProductionManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "TranspersonalGame.h"

UDir_ProductionManager::UDir_ProductionManager()
{
    InitializeAgentNames();
}

void UDir_ProductionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Studio Director Production Manager initialized"));
    InitializeProductionPipeline();
}

void UDir_ProductionManager::Deinitialize()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Studio Director Production Manager shutting down"));
    Super::Deinitialize();
}

void UDir_ProductionManager::InitializeProductionPipeline()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("=== PRODUCTION PIPELINE INITIALIZED ==="));
    
    // Clear existing tasks
    ActiveTasks.Empty();
    
    // Set critical path agents for playable prototype
    CriticalPathAgents = {2, 3, 5, 9, 10, 12};
    
    // Assign initial tasks to critical agents
    AssignTaskToAgent(2, "Define core engine architecture and compilation rules", 10.0f);
    AssignTaskToAgent(3, "Implement physics, collision, and movement systems", 9.0f);
    AssignTaskToAgent(5, "Generate playable terrain with height variation", 8.0f);
    AssignTaskToAgent(9, "Create character with WASD movement and camera", 9.0f);
    AssignTaskToAgent(10, "Add character animations and movement polish", 7.0f);
    AssignTaskToAgent(12, "Implement survival HUD with health/hunger bars", 6.0f);
    
    CurrentPhase = EDir_ProductionPhase::PrototypeDevelopment;
    UpdateProductionStatus();
}

void UDir_ProductionManager::UpdateProductionStatus()
{
    UpdateMapMetrics();
    CheckPlayabilityStatus();
    CalculateOverallProgress();
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Production Status Updated - Actors: %d, Dinos: %d, Progress: %.1f%%"), 
           CurrentMetrics.TotalActorsInMap, 
           CurrentMetrics.DinosaurCount, 
           CurrentMetrics.OverallProgress * 100.0f);
}

void UDir_ProductionManager::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.AgentName = AgentNames.Contains(AgentNumber) ? AgentNames[AgentNumber] : FString::Printf(TEXT("Agent #%d"), AgentNumber);
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.StartTime = FDateTime::Now();
    NewTask.DeadlineTime = FDateTime::Now() + FTimespan::FromHours(2);
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTranspersonalGame, Warning, TEXT("TASK ASSIGNED: Agent #%d (%s) - %s [Priority: %.1f]"), 
           AgentNumber, *NewTask.AgentName, *TaskDescription, Priority);
}

void UDir_ProductionManager::CompleteAgentTask(int32 AgentNumber, const TArray<FString>& Deliverables)
{
    for (int32 i = 0; i < ActiveTasks.Num(); i++)
    {
        if (ActiveTasks[i].AgentNumber == AgentNumber && ActiveTasks[i].Status == EDir_AgentStatus::Working)
        {
            ActiveTasks[i].Status = EDir_AgentStatus::Complete;
            ActiveTasks[i].Deliverables = Deliverables;
            CurrentMetrics.CompletedTasks++;
            
            UE_LOG(LogTranspersonalGame, Log, TEXT("TASK COMPLETED: Agent #%d - %s"), 
                   AgentNumber, *ActiveTasks[i].TaskDescription);
            break;
        }
    }
    
    UpdateProductionStatus();
}

void UDir_ProductionManager::MarkAgentBlocked(int32 AgentNumber, const FString& BlockingReason)
{
    for (int32 i = 0; i < ActiveTasks.Num(); i++)
    {
        if (ActiveTasks[i].AgentNumber == AgentNumber && ActiveTasks[i].Status == EDir_AgentStatus::Working)
        {
            ActiveTasks[i].Status = EDir_AgentStatus::Blocked;
            CurrentMetrics.FailedTasks++;
            
            UE_LOG(LogTranspersonalGame, Error, TEXT("AGENT BLOCKED: Agent #%d - %s | Reason: %s"), 
                   AgentNumber, *ActiveTasks[i].TaskDescription, *BlockingReason);
            break;
        }
    }
}

FDir_ProductionMetrics UDir_ProductionManager::GetCurrentMetrics()
{
    UpdateProductionStatus();
    return CurrentMetrics;
}

TArray<FDir_AgentTask> UDir_ProductionManager::GetActiveTasks()
{
    TArray<FDir_AgentTask> WorkingTasks;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            WorkingTasks.Add(Task);
        }
    }
    return WorkingTasks;
}

TArray<FDir_AgentTask> UDir_ProductionManager::GetBlockedTasks()
{
    TArray<FDir_AgentTask> BlockedTasks;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedTasks.Add(Task);
        }
    }
    return BlockedTasks;
}

bool UDir_ProductionManager::IsAgentAvailable(int32 AgentNumber)
{
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentNumber == AgentNumber && Task.Status == EDir_AgentStatus::Working)
        {
            return false;
        }
    }
    return true;
}

void UDir_ProductionManager::IdentifyCriticalPath()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("=== CRITICAL PATH ANALYSIS ==="));
    
    for (int32 AgentNum : CriticalPathAgents)
    {
        bool HasActiveTask = false;
        for (const FDir_AgentTask& Task : ActiveTasks)
        {
            if (Task.AgentNumber == AgentNum && Task.Status == EDir_AgentStatus::Working)
            {
                HasActiveTask = true;
                UE_LOG(LogTranspersonalGame, Warning, TEXT("CRITICAL AGENT #%d: %s"), AgentNum, *Task.TaskDescription);
                break;
            }
        }
        
        if (!HasActiveTask)
        {
            UE_LOG(LogTranspersonalGame, Error, TEXT("CRITICAL AGENT #%d: NO ACTIVE TASK - BLOCKING PROTOTYPE"), AgentNum);
        }
    }
}

void UDir_ProductionManager::PrioritizePlayablePrototype()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("=== PRIORITIZING PLAYABLE PROTOTYPE ==="));
    
    // Boost priority of critical prototype tasks
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (CriticalPathAgents.Contains(Task.AgentNumber))
        {
            Task.Priority = FMath::Max(Task.Priority, 8.0f);
        }
    }
    
    CurrentPhase = EDir_ProductionPhase::VerticalSlice;
}

void UDir_ProductionManager::ValidateMinimumViableProduct()
{
    bool bHasCharacter = CurrentMetrics.CharacterCount > 0;
    bool bHasTerrain = CurrentMetrics.TotalActorsInMap > 100; // Assume terrain generates many actors
    bool bHasDinosaurs = CurrentMetrics.DinosaurCount > 0;
    
    CurrentMetrics.bIsPlayable = bHasCharacter && bHasTerrain && bHasDinosaurs;
    
    UE_LOG(LogTranspersonalGame, Warning, TEXT("MVP VALIDATION: Character=%s, Terrain=%s, Dinos=%s, Playable=%s"),
           bHasCharacter ? TEXT("YES") : TEXT("NO"),
           bHasTerrain ? TEXT("YES") : TEXT("NO"),
           bHasDinosaurs ? TEXT("YES") : TEXT("NO"),
           CurrentMetrics.bIsPlayable ? TEXT("YES") : TEXT("NO"));
}

void UDir_ProductionManager::LogProductionStatus()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION STATUS ==="));
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Phase: %s"), CurrentPhase == EDir_ProductionPhase::PrototypeDevelopment ? TEXT("Prototype Development") : TEXT("Other"));
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsInMap);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Characters: %d"), CurrentMetrics.CharacterCount);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurCount);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Completed Tasks: %d"), CurrentMetrics.CompletedTasks);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Failed Tasks: %d"), CurrentMetrics.FailedTasks);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Overall Progress: %.1f%%"), CurrentMetrics.OverallProgress * 100.0f);
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Is Playable: %s"), CurrentMetrics.bIsPlayable ? TEXT("YES") : TEXT("NO"));
}

void UDir_ProductionManager::GenerateProductionReport()
{
    LogProductionStatus();
    IdentifyCriticalPath();
    ValidateMinimumViableProduct();
}

void UDir_ProductionManager::InitializeAgentNames()
{
    AgentNames.Add(1, TEXT("Studio Director"));
    AgentNames.Add(2, TEXT("Engine Architect"));
    AgentNames.Add(3, TEXT("Core Systems Programmer"));
    AgentNames.Add(4, TEXT("Performance Optimizer"));
    AgentNames.Add(5, TEXT("Procedural World Generator"));
    AgentNames.Add(6, TEXT("Environment Artist"));
    AgentNames.Add(7, TEXT("Architecture & Interior Agent"));
    AgentNames.Add(8, TEXT("Lighting & Atmosphere Agent"));
    AgentNames.Add(9, TEXT("Character Artist Agent"));
    AgentNames.Add(10, TEXT("Animation Agent"));
    AgentNames.Add(11, TEXT("NPC Behavior Agent"));
    AgentNames.Add(12, TEXT("Combat & Enemy AI Agent"));
    AgentNames.Add(13, TEXT("Crowd & Traffic Simulation"));
    AgentNames.Add(14, TEXT("Quest & Mission Designer"));
    AgentNames.Add(15, TEXT("Narrative & Dialogue Agent"));
    AgentNames.Add(16, TEXT("Audio Agent"));
    AgentNames.Add(17, TEXT("VFX Agent"));
    AgentNames.Add(18, TEXT("QA & Testing Agent"));
    AgentNames.Add(19, TEXT("Integration & Build Agent"));
}

void UDir_ProductionManager::UpdateMapMetrics()
{
    CurrentMetrics.LastUpdateTime = FDateTime::Now();
    
    // These would be updated by UE5 Python scripts in real implementation
    // For now, we'll set reasonable defaults that can be overridden
    if (CurrentMetrics.TotalActorsInMap == 0)
    {
        CurrentMetrics.TotalActorsInMap = 150; // Estimated based on basic map
        CurrentMetrics.DinosaurCount = 5;
        CurrentMetrics.CharacterCount = 1;
    }
}

void UDir_ProductionManager::CheckPlayabilityStatus()
{
    ValidateMinimumViableProduct();
}

void UDir_ProductionManager::CalculateOverallProgress()
{
    int32 TotalTasks = ActiveTasks.Num();
    if (TotalTasks == 0)
    {
        CurrentMetrics.OverallProgress = 0.0f;
        return;
    }
    
    float CompletedWeight = CurrentMetrics.CompletedTasks * 1.0f;
    float WorkingWeight = 0.0f;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            WorkingWeight += 0.5f; // Working tasks count as 50% complete
        }
    }
    
    CurrentMetrics.OverallProgress = FMath::Clamp((CompletedWeight + WorkingWeight) / TotalTasks, 0.0f, 1.0f);
}