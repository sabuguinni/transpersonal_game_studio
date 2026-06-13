#include "ProductionCoordinator.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    CurrentPhase = EDir_ProductionPhase::CoreSystems;
    bAutoUpdateMetrics = true;
    MetricsUpdateInterval = 5.0f;
    LastMetricsUpdate = 0.0f;
    
    // Initialize production metrics
    ProductionMetrics = FDir_ProductionMetrics();
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionChain();
    UpdateProductionMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: System initialized for %s"), *ProductionMetrics.CurrentCycle);
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoUpdateMetrics)
    {
        LastMetricsUpdate += DeltaTime;
        if (LastMetricsUpdate >= MetricsUpdateInterval)
        {
            UpdateMetricsInternal();
            LastMetricsUpdate = 0.0f;
        }
    }
}

void AProductionCoordinator::InitializeProductionChain()
{
    AgentTasks.Empty();
    
    // Define the complete agent chain with dependencies
    TArray<FDir_AgentTask> InitialTasks;
    
    // Agent #02 - Engine Architect (no dependencies)
    FDir_AgentTask EngineTask;
    EngineTask.AgentName = TEXT("Agent_02_Engine_Architect");
    EngineTask.TaskDescription = TEXT("Define core architecture and technical specifications");
    EngineTask.Status = EDir_AgentStatus::Working;
    EngineTask.Priority = 10.0f;
    EngineTask.FilesRequired = 8;
    InitialTasks.Add(EngineTask);
    
    // Agent #03 - Core Systems (depends on Engine)
    FDir_AgentTask CoreTask;
    CoreTask.AgentName = TEXT("Agent_03_Core_Systems");
    CoreTask.TaskDescription = TEXT("Implement physics, collision, and core gameplay systems");
    CoreTask.Status = EDir_AgentStatus::Idle;
    CoreTask.Priority = 9.0f;
    CoreTask.Dependencies.Add(TEXT("Agent_02_Engine_Architect"));
    CoreTask.FilesRequired = 8;
    InitialTasks.Add(CoreTask);
    
    // Agent #04 - Performance Optimizer (depends on Core)
    FDir_AgentTask PerfTask;
    PerfTask.AgentName = TEXT("Agent_04_Performance");
    PerfTask.TaskDescription = TEXT("Optimize for 60fps and implement performance monitoring");
    PerfTask.Status = EDir_AgentStatus::Idle;
    PerfTask.Priority = 8.0f;
    PerfTask.Dependencies.Add(TEXT("Agent_03_Core_Systems"));
    PerfTask.FilesRequired = 8;
    InitialTasks.Add(PerfTask);
    
    // Agent #05 - World Generator (depends on Performance)
    FDir_AgentTask WorldTask;
    WorldTask.AgentName = TEXT("Agent_05_World_Generator");
    WorldTask.TaskDescription = TEXT("Create procedural terrain and biome systems");
    WorldTask.Status = EDir_AgentStatus::Idle;
    WorldTask.Priority = 7.0f;
    WorldTask.Dependencies.Add(TEXT("Agent_04_Performance"));
    WorldTask.FilesRequired = 8;
    InitialTasks.Add(WorldTask);
    
    // Agent #06 - Environment Artist (depends on World)
    FDir_AgentTask EnvTask;
    EnvTask.AgentName = TEXT("Agent_06_Environment");
    EnvTask.TaskDescription = TEXT("Populate world with vegetation, rocks, and environmental assets");
    EnvTask.Status = EDir_AgentStatus::Idle;
    EnvTask.Priority = 6.0f;
    EnvTask.Dependencies.Add(TEXT("Agent_05_World_Generator"));
    EnvTask.FilesRequired = 8;
    InitialTasks.Add(EnvTask);
    
    // Agent #09 - Character Artist (parallel to Environment)
    FDir_AgentTask CharTask;
    CharTask.AgentName = TEXT("Agent_09_Character");
    CharTask.TaskDescription = TEXT("Create player characters and NPCs using MetaHuman");
    CharTask.Status = EDir_AgentStatus::Idle;
    CharTask.Priority = 6.0f;
    CharTask.Dependencies.Add(TEXT("Agent_03_Core_Systems"));
    CharTask.FilesRequired = 8;
    InitialTasks.Add(CharTask);
    
    // Agent #10 - Animation (depends on Character)
    FDir_AgentTask AnimTask;
    AnimTask.AgentName = TEXT("Agent_10_Animation");
    AnimTask.TaskDescription = TEXT("Implement Motion Matching and IK systems");
    AnimTask.Status = EDir_AgentStatus::Idle;
    AnimTask.Priority = 5.0f;
    AnimTask.Dependencies.Add(TEXT("Agent_09_Character"));
    AnimTask.FilesRequired = 8;
    InitialTasks.Add(AnimTask);
    
    // Agent #12 - Combat AI (depends on Animation)
    FDir_AgentTask CombatTask;
    CombatTask.AgentName = TEXT("Agent_12_Combat");
    CombatTask.TaskDescription = TEXT("Implement dinosaur AI and combat mechanics");
    CombatTask.Status = EDir_AgentStatus::Idle;
    CombatTask.Priority = 4.0f;
    CombatTask.Dependencies.Add(TEXT("Agent_10_Animation"));
    CombatTask.FilesRequired = 8;
    InitialTasks.Add(CombatTask);
    
    AgentTasks = InitialTasks;
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d agent tasks"), AgentTasks.Num());
}

void AProductionCoordinator::UpdateProductionMetrics()
{
    ProductionMetrics.TotalTasks = AgentTasks.Num();
    ProductionMetrics.ActiveAgents = 0;
    ProductionMetrics.CompletedTasks = 0;
    ProductionMetrics.TotalFilesCreated = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            ProductionMetrics.ActiveAgents++;
        }
        else if (Task.Status == EDir_AgentStatus::Complete)
        {
            ProductionMetrics.CompletedTasks++;
        }
        
        ProductionMetrics.TotalFilesCreated += Task.FilesCreated;
    }
    
    if (ProductionMetrics.TotalTasks > 0)
    {
        ProductionMetrics.OverallProgress = (float)ProductionMetrics.CompletedTasks / (float)ProductionMetrics.TotalTasks * 100.0f;
    }
    
    LogProductionState();
}

void AProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.TaskDescription = TaskDescription;
            Task.Priority = Priority;
            Task.Status = EDir_AgentStatus::Working;
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Assigned task to %s: %s"), *AgentName, *TaskDescription);
            return;
        }
    }
    
    // Create new task if agent not found
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Created new task for %s: %s"), *AgentName, *TaskDescription);
}

void AProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Updated %s status to %d"), *AgentName, (int32)NewStatus);
            
            // Check if this unblocks other agents
            if (NewStatus == EDir_AgentStatus::Complete)
            {
                CheckDependencies();
            }
            return;
        }
    }
}

void AProductionCoordinator::CheckDependencies()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle)
        {
            bool CanProceed = true;
            
            for (const FString& Dependency : Task.Dependencies)
            {
                bool DependencyMet = false;
                for (const FDir_AgentTask& DepTask : AgentTasks)
                {
                    if (DepTask.AgentName == Dependency && DepTask.Status == EDir_AgentStatus::Complete)
                    {
                        DependencyMet = true;
                        break;
                    }
                }
                
                if (!DependencyMet)
                {
                    CanProceed = false;
                    break;
                }
            }
            
            if (CanProceed)
            {
                Task.Status = EDir_AgentStatus::Working;
                UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: %s dependencies met, status changed to Working"), *Task.AgentName);
            }
        }
    }
}

void AProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s"), *ProductionMetrics.CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), ProductionMetrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d/%d"), ProductionMetrics.ActiveAgents, ProductionMetrics.TotalAgents);
    UE_LOG(LogTemp, Warning, TEXT("Files Created: %d/%d"), ProductionMetrics.TotalFilesCreated, ProductionMetrics.TargetFileCount);
    
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT STATUS ==="));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusStr;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusStr = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusStr = TEXT("WORKING"); break;
            case EDir_AgentStatus::Blocked: StatusStr = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Complete: StatusStr = TEXT("COMPLETE"); break;
            case EDir_AgentStatus::Error: StatusStr = TEXT("ERROR"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s (Priority: %.1f, Files: %d/%d)"), 
               *Task.AgentName, *StatusStr, Task.Priority, Task.FilesCreated, Task.FilesRequired);
    }
}

float AProductionCoordinator::GetOverallProgress() const
{
    return ProductionMetrics.OverallProgress;
}

TArray<FString> AProductionCoordinator::GetBlockedAgents() const
{
    TArray<FString> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || Task.Status == EDir_AgentStatus::Error)
        {
            BlockedAgents.Add(Task.AgentName);
        }
    }
    
    return BlockedAgents;
}

bool AProductionCoordinator::CanAgentProceed(const FString& AgentName) const
{
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            // Check if all dependencies are met
            for (const FString& Dependency : Task.Dependencies)
            {
                bool DependencyMet = false;
                for (const FDir_AgentTask& DepTask : AgentTasks)
                {
                    if (DepTask.AgentName == Dependency && DepTask.Status == EDir_AgentStatus::Complete)
                    {
                        DependencyMet = true;
                        break;
                    }
                }
                
                if (!DependencyMet)
                {
                    return false;
                }
            }
            
            return true;
        }
    }
    
    return false;
}

void AProductionCoordinator::UpdateMetricsInternal()
{
    UpdateProductionMetrics();
}

void AProductionCoordinator::ValidateAgentChain()
{
    // Validate that the agent chain is properly configured
    TArray<FString> RequiredAgents = {
        TEXT("Agent_02_Engine_Architect"),
        TEXT("Agent_03_Core_Systems"),
        TEXT("Agent_04_Performance"),
        TEXT("Agent_05_World_Generator"),
        TEXT("Agent_06_Environment"),
        TEXT("Agent_09_Character"),
        TEXT("Agent_10_Animation"),
        TEXT("Agent_12_Combat")
    };
    
    for (const FString& RequiredAgent : RequiredAgents)
    {
        bool Found = false;
        for (const FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.AgentName == RequiredAgent)
            {
                Found = true;
                break;
            }
        }
        
        if (!Found)
        {
            UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Missing required agent %s"), *RequiredAgent);
        }
    }
}

void AProductionCoordinator::LogProductionState()
{
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Progress %.1f%% | Active: %d | Files: %d/%d"), 
           ProductionMetrics.OverallProgress, ProductionMetrics.ActiveAgents, 
           ProductionMetrics.TotalFilesCreated, ProductionMetrics.TargetFileCount);
}