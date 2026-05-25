#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    CurrentProductionPhase = EDir_ProductionPhase::PreProduction;
}

void UDir_ProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized"));
    InitializeProductionPipeline();
}

void UDir_ProductionCoordinator::Deinitialize()
{
    ActiveTasks.Empty();
    AgentOutputs.Empty();
    BlockedAgents.Empty();
    
    Super::Deinitialize();
}

void UDir_ProductionCoordinator::InitializeProductionPipeline()
{
    ActiveTasks.Empty();
    AgentOutputs.Empty();
    BlockedAgents.Empty();
    
    // Setup initial tasks for each agent in the 19-agent pipeline
    TArray<FDir_AgentTask> InitialTasks;
    
    // Agent #1 - Studio Director (self)
    FDir_AgentTask DirectorTask;
    DirectorTask.AgentID = 1;
    DirectorTask.AgentName = TEXT("Studio Director");
    DirectorTask.TaskDescription = TEXT("Coordinate production pipeline and manage agent dependencies");
    DirectorTask.RequiredPhase = EDir_ProductionPhase::PreProduction;
    DirectorTask.bIsBlocking = false;
    DirectorTask.Priority = 10.0f;
    InitialTasks.Add(DirectorTask);
    
    // Agent #2 - Engine Architect
    FDir_AgentTask ArchitectTask;
    ArchitectTask.AgentID = 2;
    ArchitectTask.AgentName = TEXT("Engine Architect");
    ArchitectTask.TaskDescription = TEXT("Define technical architecture and establish coding standards");
    ArchitectTask.RequiredPhase = EDir_ProductionPhase::PreProduction;
    ArchitectTask.bIsBlocking = true;
    ArchitectTask.Priority = 9.0f;
    InitialTasks.Add(ArchitectTask);
    
    // Agent #3 - Core Systems Programmer
    FDir_AgentTask CoreSystemsTask;
    CoreSystemsTask.AgentID = 3;
    CoreSystemsTask.AgentName = TEXT("Core Systems Programmer");
    CoreSystemsTask.TaskDescription = TEXT("Implement physics, collision, and movement systems");
    CoreSystemsTask.RequiredPhase = EDir_ProductionPhase::CoreSystems;
    CoreSystemsTask.bIsBlocking = true;
    CoreSystemsTask.Priority = 8.0f;
    InitialTasks.Add(CoreSystemsTask);
    
    // Agent #4 - Performance Optimizer
    FDir_AgentTask PerformanceTask;
    PerformanceTask.AgentID = 4;
    PerformanceTask.AgentName = TEXT("Performance Optimizer");
    PerformanceTask.TaskDescription = TEXT("Ensure 60fps PC / 30fps console performance");
    PerformanceTask.RequiredPhase = EDir_ProductionPhase::CoreSystems;
    PerformanceTask.bIsBlocking = false;
    PerformanceTask.Priority = 7.0f;
    InitialTasks.Add(PerformanceTask);
    
    // Agent #5 - Procedural World Generator
    FDir_AgentTask WorldGenTask;
    WorldGenTask.AgentID = 5;
    WorldGenTask.AgentName = TEXT("Procedural World Generator");
    WorldGenTask.TaskDescription = TEXT("Generate terrain, biomes, and geographical structure");
    WorldGenTask.RequiredPhase = EDir_ProductionPhase::WorldBuilding;
    WorldGenTask.bIsBlocking = true;
    WorldGenTask.Priority = 8.0f;
    InitialTasks.Add(WorldGenTask);
    
    // Continue with remaining agents...
    for (int32 AgentID = 6; AgentID <= 19; ++AgentID)
    {
        FDir_AgentTask Task;
        Task.AgentID = AgentID;
        Task.AgentName = GetAgentNameByID(AgentID);
        Task.TaskDescription = GetInitialTaskForAgent(AgentID);
        Task.RequiredPhase = GetRequiredPhaseForAgent(AgentID);
        Task.bIsBlocking = IsAgentBlocking(AgentID);
        Task.Priority = GetAgentPriority(AgentID);
        InitialTasks.Add(Task);
    }
    
    ActiveTasks = InitialTasks;
    SetupAgentDependencies();
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline initialized with %d tasks"), ActiveTasks.Num());
}

void UDir_ProductionCoordinator::AddAgentTask(const FDir_AgentTask& Task)
{
    ActiveTasks.Add(Task);
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Task added for Agent #%d: %s"), Task.AgentID, *Task.TaskDescription);
}

void UDir_ProductionCoordinator::CompleteAgentTask(int32 AgentID, const FString& TaskDescription)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID && Task.TaskDescription == TaskDescription)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Task completed for Agent #%d: %s"), AgentID, *TaskDescription);
            break;
        }
    }
    
    // Remove from blocked agents if applicable
    BlockedAgents.Remove(AgentID);
    
    UpdateProductionMetrics();
    ValidateTaskSequence();
}

bool UDir_ProductionCoordinator::CanAgentProceed(int32 AgentID) const
{
    // Check if agent is blocked
    if (BlockedAgents.Contains(AgentID))
    {
        return false;
    }
    
    // Check if dependencies are complete
    return CheckDependenciesComplete(AgentID);
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetPendingTasksForAgent(int32 AgentID) const
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID && !Task.bIsCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    return PendingTasks;
}

FDir_ProductionMetrics UDir_ProductionCoordinator::GetProductionMetrics() const
{
    FDir_ProductionMetrics Metrics;
    Metrics.CurrentPhase = CurrentProductionPhase;
    Metrics.TotalTasks = ActiveTasks.Num();
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.bIsCompleted)
        {
            Metrics.CompletedTasks++;
        }
        if (Task.bIsBlocking && !Task.bIsCompleted)
        {
            Metrics.BlockingTasks++;
        }
    }
    
    if (Metrics.TotalTasks > 0)
    {
        Metrics.OverallProgress = (float)Metrics.CompletedTasks / (float)Metrics.TotalTasks;
    }
    
    return Metrics;
}

void UDir_ProductionCoordinator::AdvanceProductionPhase()
{
    // Check if current phase is complete
    bool bPhaseComplete = true;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.RequiredPhase == CurrentProductionPhase && !Task.bIsCompleted && Task.bIsBlocking)
        {
            bPhaseComplete = false;
            break;
        }
    }
    
    if (bPhaseComplete)
    {
        switch (CurrentProductionPhase)
        {
            case EDir_ProductionPhase::PreProduction:
                CurrentProductionPhase = EDir_ProductionPhase::CoreSystems;
                break;
            case EDir_ProductionPhase::CoreSystems:
                CurrentProductionPhase = EDir_ProductionPhase::WorldBuilding;
                break;
            case EDir_ProductionPhase::WorldBuilding:
                CurrentProductionPhase = EDir_ProductionPhase::GameplayLoop;
                break;
            case EDir_ProductionPhase::GameplayLoop:
                CurrentProductionPhase = EDir_ProductionPhase::Polish;
                break;
            case EDir_ProductionPhase::Polish:
                CurrentProductionPhase = EDir_ProductionPhase::Release;
                break;
            default:
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Production phase advanced to: %d"), (int32)CurrentProductionPhase);
    }
}

bool UDir_ProductionCoordinator::IsPhaseBlocked() const
{
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.RequiredPhase == CurrentProductionPhase && Task.bIsBlocking && !Task.bIsCompleted)
        {
            return true;
        }
    }
    return false;
}

void UDir_ProductionCoordinator::RegisterAgentCompletion(int32 AgentID, const FString& Output)
{
    AgentOutputs.Add(AgentID, Output);
    UE_LOG(LogTemp, Log, TEXT("Agent #%d registered completion: %s"), AgentID, *Output);
}

FString UDir_ProductionCoordinator::GetNextAgentInChain(int32 CurrentAgentID) const
{
    // Simple sequential chain for now
    int32 NextAgentID = CurrentAgentID + 1;
    if (NextAgentID > 19)
    {
        NextAgentID = 1; // Loop back to Studio Director
    }
    
    return GetAgentNameByID(NextAgentID);
}

void UDir_ProductionCoordinator::LogProductionStatus()
{
    FDir_ProductionMetrics Metrics = GetProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)Metrics.CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Progress: %d/%d tasks (%.1f%%)"), 
           Metrics.CompletedTasks, Metrics.TotalTasks, Metrics.OverallProgress * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Blocking Tasks: %d"), Metrics.BlockingTasks);
    UE_LOG(LogTemp, Warning, TEXT("Blocked Agents: %d"), BlockedAgents.Num());
}

void UDir_ProductionCoordinator::ResetProductionPipeline()
{
    CurrentProductionPhase = EDir_ProductionPhase::PreProduction;
    InitializeProductionPipeline();
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline reset"));
}

void UDir_ProductionCoordinator::SetupAgentDependencies()
{
    // Engine Architect must complete before Core Systems
    // Core Systems must complete before World Building
    // etc.
    
    // For now, simple sequential dependencies
    // TODO: Implement complex dependency graph
}

void UDir_ProductionCoordinator::ValidateTaskSequence()
{
    // Check if any agents should be unblocked
    for (int32 AgentID : BlockedAgents)
    {
        if (CheckDependenciesComplete(AgentID))
        {
            BlockedAgents.Remove(AgentID);
            UE_LOG(LogTemp, Log, TEXT("Agent #%d unblocked"), AgentID);
        }
    }
}

bool UDir_ProductionCoordinator::CheckDependenciesComplete(int32 AgentID) const
{
    // Simple dependency check - previous agent must be complete
    if (AgentID == 1) return true; // Studio Director has no dependencies
    
    int32 PreviousAgentID = AgentID - 1;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == PreviousAgentID && Task.bIsBlocking && !Task.bIsCompleted)
        {
            return false;
        }
    }
    
    return true;
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    // Metrics are calculated on-demand in GetProductionMetrics()
    // This function can be used for caching if needed
}

FString UDir_ProductionCoordinator::GetAgentNameByID(int32 AgentID) const
{
    switch (AgentID)
    {
        case 1: return TEXT("Studio Director");
        case 2: return TEXT("Engine Architect");
        case 3: return TEXT("Core Systems Programmer");
        case 4: return TEXT("Performance Optimizer");
        case 5: return TEXT("Procedural World Generator");
        case 6: return TEXT("Environment Artist");
        case 7: return TEXT("Architecture & Interior Agent");
        case 8: return TEXT("Lighting & Atmosphere Agent");
        case 9: return TEXT("Character Artist Agent");
        case 10: return TEXT("Animation Agent");
        case 11: return TEXT("NPC Behavior Agent");
        case 12: return TEXT("Combat & Enemy AI Agent");
        case 13: return TEXT("Crowd & Traffic Simulation");
        case 14: return TEXT("Quest & Mission Designer");
        case 15: return TEXT("Narrative & Dialogue Agent");
        case 16: return TEXT("Audio Agent");
        case 17: return TEXT("VFX Agent");
        case 18: return TEXT("QA & Testing Agent");
        case 19: return TEXT("Integration & Build Agent");
        default: return TEXT("Unknown Agent");
    }
}

FString UDir_ProductionCoordinator::GetInitialTaskForAgent(int32 AgentID) const
{
    switch (AgentID)
    {
        case 6: return TEXT("Create environment assets and populate biomes");
        case 7: return TEXT("Build prehistoric structures and interiors");
        case 8: return TEXT("Implement day/night cycle and atmospheric lighting");
        case 9: return TEXT("Create character models and MetaHuman integration");
        case 10: return TEXT("Implement Motion Matching and IK systems");
        case 11: return TEXT("Design NPC behavior trees and daily routines");
        case 12: return TEXT("Implement dinosaur combat AI and tactical behavior");
        case 13: return TEXT("Setup Mass AI for crowd simulation");
        case 14: return TEXT("Design survival missions and objectives");
        case 15: return TEXT("Write game bible and narrative content");
        case 16: return TEXT("Implement MetaSounds and adaptive audio");
        case 17: return TEXT("Create Niagara VFX with LOD optimization");
        case 18: return TEXT("Establish QA testing framework");
        case 19: return TEXT("Setup build integration and deployment");
        default: return TEXT("Unknown task");
    }
}

EDir_ProductionPhase UDir_ProductionCoordinator::GetRequiredPhaseForAgent(int32 AgentID) const
{
    if (AgentID <= 4) return EDir_ProductionPhase::PreProduction;
    if (AgentID <= 8) return EDir_ProductionPhase::WorldBuilding;
    if (AgentID <= 13) return EDir_ProductionPhase::GameplayLoop;
    return EDir_ProductionPhase::Polish;
}

bool UDir_ProductionCoordinator::IsAgentBlocking(int32 AgentID) const
{
    // Critical path agents that block others
    TArray<int32> BlockingAgents = {2, 3, 5, 15, 18};
    return BlockingAgents.Contains(AgentID);
}

float UDir_ProductionCoordinator::GetAgentPriority(int32 AgentID) const
{
    // Higher priority for earlier agents in the chain
    return 20.0f - (float)AgentID;
}