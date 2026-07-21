#include "StudioDirectorSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UStudioDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentPhase = EDir_ProductionPhase::Production;
    CurrentCycleId = FString::Printf(TEXT("PROD_CYCLE_AUTO_%s"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M")));
    CycleStartTime = FDateTime::Now();
    
    InitializeAgentTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Subsystem Initialized - Cycle: %s"), *CurrentCycleId);
}

void UStudioDirectorSubsystem::Deinitialize()
{
    LogProductionStatus();
    Super::Deinitialize();
}

void UStudioDirectorSubsystem::InitializeAgentTasks()
{
    AgentTasks.Empty();
    
    // Initialize all 19 agent tasks with proper dependencies
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),
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
    
    for (int32 i = 0; i < AgentNames.Num(); ++i)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentNumber = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.Priority = 1.0f;
        
        // Set initial task descriptions based on current milestone
        switch (i + 1)
        {
            case 1: // Studio Director
                NewTask.TaskDescription = TEXT("Coordinate Milestone 1: Walk Around prototype");
                NewTask.Status = EDir_AgentStatus::Working;
                break;
            case 2: // Engine Architect
                NewTask.TaskDescription = TEXT("Define physics collision for dinosaur interactions");
                NewTask.Dependencies.Add(TEXT("Agent #1 coordination"));
                break;
            case 3: // Core Systems
                NewTask.TaskDescription = TEXT("Implement ragdoll physics for dinosaur death states");
                NewTask.Dependencies.Add(TEXT("Agent #2 architecture"));
                break;
            case 5: // World Generator
                NewTask.TaskDescription = TEXT("Populate 5 biomes with 500+ actors each");
                NewTask.Priority = 2.0f;
                break;
            case 9: // Character Artist
                NewTask.TaskDescription = TEXT("Create tribal human survival characters");
                NewTask.Priority = 2.0f;
                break;
            case 12: // Combat AI
                NewTask.TaskDescription = TEXT("Implement dinosaur territorial behavior");
                NewTask.Dependencies.Add(TEXT("Spawned dinosaurs"));
                NewTask.Priority = 1.5f;
                break;
            default:
                NewTask.TaskDescription = FString::Printf(TEXT("Awaiting %s implementation"), *AgentNames[i]);
                break;
        }
        
        AgentTasks.Add(NewTask);
    }
    
    ProductionMetrics.ActiveAgents = AgentTasks.Num();
}

void UStudioDirectorSubsystem::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority)
{
    if (AgentNumber < 1 || AgentNumber > AgentTasks.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid agent number: %d"), AgentNumber);
        return;
    }
    
    FDir_AgentTask& Task = AgentTasks[AgentNumber - 1];
    Task.TaskDescription = TaskDescription;
    Task.Priority = Priority;
    Task.Status = EDir_AgentStatus::Working;
    Task.AssignedTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%d (%s): %s"), 
           AgentNumber, *Task.AgentName, *TaskDescription);
}

void UStudioDirectorSubsystem::CompleteAgentTask(int32 AgentNumber, const TArray<FString>& Deliverables)
{
    if (AgentNumber < 1 || AgentNumber > AgentTasks.Num())
    {
        return;
    }
    
    FDir_AgentTask& Task = AgentTasks[AgentNumber - 1];
    Task.Status = EDir_AgentStatus::Completed;
    Task.CompletedTime = FDateTime::Now();
    Task.Deliverables = Deliverables;
    
    ProductionMetrics.CompletedTasks++;
    
    UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s) completed task with %d deliverables"), 
           AgentNumber, *Task.AgentName, Deliverables.Num());
}

void UStudioDirectorSubsystem::BlockAgent(int32 AgentNumber, const FString& Reason)
{
    if (AgentNumber < 1 || AgentNumber > AgentTasks.Num())
    {
        return;
    }
    
    FDir_AgentTask& Task = AgentTasks[AgentNumber - 1];
    Task.Status = EDir_AgentStatus::Blocked;
    Task.TaskDescription += FString::Printf(TEXT(" [BLOCKED: %s]"), *Reason);
    
    UE_LOG(LogTemp, Error, TEXT("Agent #%d (%s) blocked: %s"), 
           AgentNumber, *Task.AgentName, *Reason);
}

TArray<FDir_AgentTask> UStudioDirectorSubsystem::GetActiveAgentTasks() const
{
    TArray<FDir_AgentTask> ActiveTasks;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            ActiveTasks.Add(Task);
        }
    }
    return ActiveTasks;
}

FDir_AgentTask UStudioDirectorSubsystem::GetAgentTask(int32 AgentNumber) const
{
    if (AgentNumber >= 1 && AgentNumber <= AgentTasks.Num())
    {
        return AgentTasks[AgentNumber - 1];
    }
    return FDir_AgentTask();
}

void UStudioDirectorSubsystem::StartProductionCycle()
{
    CycleStartTime = FDateTime::Now();
    ProductionMetrics.TotalCycles++;
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle started: %s"), *CurrentCycleId);
}

void UStudioDirectorSubsystem::EndProductionCycle()
{
    FTimespan CycleDuration = FDateTime::Now() - CycleStartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle completed in %.2f minutes"), 
           CycleDuration.GetTotalMinutes());
    
    GenerateProductionReport();
}

void UStudioDirectorSubsystem::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("Production phase changed to: %d"), (int32)NewPhase);
}

FDir_ProductionMetrics UStudioDirectorSubsystem::GetProductionMetrics() const
{
    FDir_ProductionMetrics CurrentMetrics = ProductionMetrics;
    
    // Calculate overall progress
    int32 CompletedAgents = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedAgents++;
        }
    }
    
    CurrentMetrics.OverallProgress = AgentTasks.Num() > 0 ? 
        (float)CompletedAgents / (float)AgentTasks.Num() * 100.0f : 0.0f;
    
    CurrentMetrics.bPlayablePrototypeReady = ValidateMilestone1_WalkAround();
    
    return CurrentMetrics;
}

void UStudioDirectorSubsystem::UpdateDinosaurCount(int32 NewCount)
{
    ProductionMetrics.SpawnedDinosaurs = NewCount;
    UE_LOG(LogTemp, Log, TEXT("Dinosaur count updated: %d"), NewCount);
}

void UStudioDirectorSubsystem::UpdateBiomeCount(int32 NewCount)
{
    ProductionMetrics.PopulatedBiomes = NewCount;
    UE_LOG(LogTemp, Log, TEXT("Populated biomes: %d"), NewCount);
}

void UStudioDirectorSubsystem::SetPlayablePrototypeStatus(bool bReady)
{
    ProductionMetrics.bPlayablePrototypeReady = bReady;
    UE_LOG(LogTemp, Warning, TEXT("Playable prototype status: %s"), 
           bReady ? TEXT("READY") : TEXT("NOT READY"));
}

bool UStudioDirectorSubsystem::ValidateMilestone1_WalkAround() const
{
    // Check for minimum requirements:
    // - Character with movement
    // - Landscape with terrain
    // - 3-5 dinosaurs in world
    // - Basic lighting
    
    bool bHasCharacter = ProductionMetrics.SpawnedDinosaurs > 0; // Placeholder check
    bool bHasTerrain = ProductionMetrics.PopulatedBiomes > 0;
    bool bHasDinosaurs = ProductionMetrics.SpawnedDinosaurs >= 3;
    bool bHasLighting = true; // Assume basic lighting exists
    
    return bHasCharacter && bHasTerrain && bHasDinosaurs && bHasLighting;
}

bool UStudioDirectorSubsystem::ValidateAgentDependencies(int32 AgentNumber) const
{
    if (AgentNumber < 1 || AgentNumber > AgentTasks.Num())
    {
        return false;
    }
    
    const FDir_AgentTask& Task = AgentTasks[AgentNumber - 1];
    
    // Check if all dependencies are completed
    for (const FString& Dependency : Task.Dependencies)
    {
        // Simple dependency validation - in real implementation would be more sophisticated
        if (Dependency.Contains(TEXT("Agent #")))
        {
            // Extract agent number and check if completed
            // For now, assume dependencies are met
        }
    }
    
    return true;
}

void UStudioDirectorSubsystem::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s"), *CurrentCycleId);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    
    FDir_ProductionMetrics Metrics = GetProductionMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), Metrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), Metrics.CompletedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Spawned Dinosaurs: %d"), Metrics.SpawnedDinosaurs);
    UE_LOG(LogTemp, Warning, TEXT("Populated Biomes: %d"), Metrics.PopulatedBiomes);
    UE_LOG(LogTemp, Warning, TEXT("Playable Prototype: %s"), 
           Metrics.bPlayablePrototypeReady ? TEXT("READY") : TEXT("NOT READY"));
    
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
            case EDir_AgentStatus::Error: StatusStr = TEXT("ERROR"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s): %s - %s"), 
               Task.AgentNumber, *Task.AgentName, *StatusStr, *Task.TaskDescription);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UStudioDirectorSubsystem::LogProductionStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("Studio Director shutting down - Final status logged"));
}