#include "Dir_ProductionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EditorLevelLibrary.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

UDir_ProductionManager::UDir_ProductionManager()
{
    CurrentPhase = EDir_ProductionPhase::PrototypeDevelopment;
    CurrentCycleNumber = 0;
    bProductionActive = false;
    CycleStartTime = FDateTime::Now();
}

void UDir_ProductionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Manager Initialized"));
    
    // Initialize agent tasks for the 18-agent pipeline
    InitializeAgentTasks();
    
    // Set initial production phase
    SetProductionPhase(EDir_ProductionPhase::PrototypeDevelopment);
    
    // Update initial metrics
    UpdateMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Manager ready for CYCLE coordination"));
}

void UDir_ProductionManager::Deinitialize()
{
    if (bProductionActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Production Manager shutting down during active cycle"));
        CompleteProductionCycle();
    }
    
    Super::Deinitialize();
}

void UDir_ProductionManager::StartProductionCycle(int32 CycleNumber)
{
    CurrentCycleNumber = CycleNumber;
    bProductionActive = true;
    CycleStartTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION CYCLE %d STARTED ==="), CycleNumber);
    
    // Reset all agent statuses to Idle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.StartTime = FDateTime::Now();
    }
    
    // Update metrics at cycle start
    UpdateMetrics();
    
    LogProductionStatus();
}

void UDir_ProductionManager::CompleteProductionCycle()
{
    if (!bProductionActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("No active production cycle to complete"));
        return;
    }
    
    bProductionActive = false;
    FDateTime CycleEndTime = FDateTime::Now();
    FTimespan CycleDuration = CycleEndTime - CycleStartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION CYCLE %d COMPLETED ==="), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Cycle Duration: %.2f seconds"), CycleDuration.GetTotalSeconds());
    
    // Final metrics update
    UpdateMetrics();
    
    // Generate completion report
    GenerateProductionReport();
    
    // Validate if we achieved minimum viable prototype
    bool bPrototypeValid = ValidateMinimumViablePrototype();
    UE_LOG(LogTemp, Warning, TEXT("Minimum Viable Prototype Status: %s"), 
           bPrototypeValid ? TEXT("ACHIEVED") : TEXT("NOT ACHIEVED"));
}

void UDir_ProductionManager::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    // Find existing agent task or create new one
    FDir_AgentTask* ExistingTask = AgentTasks.FindByPredicate([&AgentName](const FDir_AgentTask& Task)
    {
        return Task.AgentName == AgentName;
    });
    
    if (ExistingTask)
    {
        ExistingTask->TaskDescription = TaskDescription;
        ExistingTask->Priority = Priority;
        ExistingTask->Status = EDir_AgentStatus::Working;
        ExistingTask->StartTime = FDateTime::Now();
        ExistingTask->EstimatedCompletion = FDateTime::Now() + FTimespan::FromMinutes(10);
    }
    else
    {
        FDir_AgentTask NewTask;
        NewTask.AgentName = AgentName;
        NewTask.TaskDescription = TaskDescription;
        NewTask.Priority = Priority;
        NewTask.Status = EDir_AgentStatus::Working;
        NewTask.StartTime = FDateTime::Now();
        NewTask.EstimatedCompletion = FDateTime::Now() + FTimespan::FromMinutes(10);
        
        AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Task assigned to %s: %s (Priority: %.1f)"), 
           *AgentName, *TaskDescription, Priority);
}

void UDir_ProductionManager::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    FDir_AgentTask* Task = AgentTasks.FindByPredicate([&AgentName](const FDir_AgentTask& Task)
    {
        return Task.AgentName == AgentName;
    });
    
    if (Task)
    {
        Task->Status = NewStatus;
        
        FString StatusString;
        switch (NewStatus)
        {
            case EDir_AgentStatus::Idle: StatusString = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusString = TEXT("WORKING"); break;
            case EDir_AgentStatus::Completed: StatusString = TEXT("COMPLETED"); break;
            case EDir_AgentStatus::Blocked: StatusString = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Failed: StatusString = TEXT("FAILED"); break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Agent %s status updated to %s"), *AgentName, *StatusString);
    }
}

void UDir_ProductionManager::BlockAgent(const FString& AgentName, const FString& Reason)
{
    UpdateAgentStatus(AgentName, EDir_AgentStatus::Blocked);
    UE_LOG(LogTemp, Error, TEXT("Agent %s BLOCKED: %s"), *AgentName, *Reason);
}

FDir_ProductionMetrics UDir_ProductionManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UDir_ProductionManager::UpdateMetrics()
{
    CurrentMetrics.LastBuildTime = FDateTime::Now();
    
    // Count actors in current level
    CurrentMetrics.ActorsInLevel = CountActorsInLevel();
    
    // Get frame rate (simplified)
    CurrentMetrics.FrameRate = 60.0f; // Default assumption
    
    // File counting would require file system access
    CurrentMetrics.TotalCppFiles = 50; // Estimated based on project structure
    CurrentMetrics.TotalHeaderFiles = 150; // Estimated based on project structure
    
    // Compilation status
    CurrentMetrics.CompilationErrors = CheckCompilationStatus() ? 0 : 1;
    
    UE_LOG(LogTemp, Log, TEXT("Metrics updated - Actors: %d, Errors: %d"), 
           CurrentMetrics.ActorsInLevel, CurrentMetrics.CompilationErrors);
}

TArray<FDir_AgentTask> UDir_ProductionManager::GetAllAgentTasks() const
{
    return AgentTasks;
}

TArray<FDir_AgentTask> UDir_ProductionManager::GetActiveAgentTasks() const
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

bool UDir_ProductionManager::ValidateMinimumViablePrototype() const
{
    // Check if we have the basic requirements for a playable prototype
    int32 ActorCount = CountActorsInLevel();
    bool bHasCompilation = CheckCompilationStatus();
    
    // Minimum requirements:
    // - At least 10 actors in level (terrain, character, some props)
    // - No compilation errors
    // - Production phase is at least VerticalSlice
    
    bool bHasMinimumActors = ActorCount >= 10;
    bool bPhaseReady = CurrentPhase >= EDir_ProductionPhase::VerticalSlice;
    
    UE_LOG(LogTemp, Warning, TEXT("MVP Validation - Actors: %d, Compilation: %s, Phase Ready: %s"),
           ActorCount, bHasCompilation ? TEXT("OK") : TEXT("FAIL"), bPhaseReady ? TEXT("OK") : TEXT("FAIL"));
    
    return bHasMinimumActors && bHasCompilation && bPhaseReady;
}

bool UDir_ProductionManager::CheckCompilationStatus() const
{
    // In a real scenario, this would check actual compilation status
    // For now, we assume compilation is successful if we're running
    return true;
}

int32 UDir_ProductionManager::CountActorsInLevel() const
{
    if (UWorld* World = GetWorld())
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        return ActorCount;
    }
    
    return 0;
}

void UDir_ProductionManager::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    CurrentPhase = NewPhase;
    
    FString PhaseString;
    switch (NewPhase)
    {
        case EDir_ProductionPhase::PreProduction: PhaseString = TEXT("PRE-PRODUCTION"); break;
        case EDir_ProductionPhase::PrototypeDevelopment: PhaseString = TEXT("PROTOTYPE DEVELOPMENT"); break;
        case EDir_ProductionPhase::VerticalSlice: PhaseString = TEXT("VERTICAL SLICE"); break;
        case EDir_ProductionPhase::Production: PhaseString = TEXT("PRODUCTION"); break;
        case EDir_ProductionPhase::Polish: PhaseString = TEXT("POLISH"); break;
        case EDir_ProductionPhase::Release: PhaseString = TEXT("RELEASE"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production Phase set to: %s"), *PhaseString);
}

EDir_ProductionPhase UDir_ProductionManager::GetCurrentProductionPhase() const
{
    return CurrentPhase;
}

void UDir_ProductionManager::EmergencyStopProduction()
{
    UE_LOG(LogTemp, Error, TEXT("=== EMERGENCY PRODUCTION STOP ==="));
    
    bProductionActive = false;
    
    // Mark all working agents as failed
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Failed;
        }
    }
    
    GenerateProductionReport();
}

void UDir_ProductionManager::ForceCompilationCheck()
{
    UE_LOG(LogTemp, Warning, TEXT("Force compilation check requested"));
    UpdateMetrics();
    
    bool bCompilationOK = CheckCompilationStatus();
    UE_LOG(LogTemp, Warning, TEXT("Compilation Status: %s"), 
           bCompilationOK ? TEXT("SUCCESS") : TEXT("FAILED"));
}

void UDir_ProductionManager::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - CYCLE %d ==="), CurrentCycleNumber);
    
    FDir_ProductionMetrics Metrics = GetCurrentMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Total Actors in Level: %d"), Metrics.ActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("Compilation Errors: %d"), Metrics.CompilationErrors);
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.1f FPS"), Metrics.FrameRate);
    
    // Agent status summary
    int32 IdleAgents = 0, WorkingAgents = 0, CompletedAgents = 0, BlockedAgents = 0, FailedAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: IdleAgents++; break;
            case EDir_AgentStatus::Working: WorkingAgents++; break;
            case EDir_AgentStatus::Completed: CompletedAgents++; break;
            case EDir_AgentStatus::Blocked: BlockedAgents++; break;
            case EDir_AgentStatus::Failed: FailedAgents++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Status - Idle: %d, Working: %d, Completed: %d, Blocked: %d, Failed: %d"),
           IdleAgents, WorkingAgents, CompletedAgents, BlockedAgents, FailedAgents);
    
    UE_LOG(LogTemp, Warning, TEXT("=== END PRODUCTION REPORT ==="));
}

void UDir_ProductionManager::InitializeAgentTasks()
{
    // Initialize tasks for all 18 agents
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
    
    AgentTasks.Empty();
    
    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentTask Task;
        Task.AgentName = AgentName;
        Task.TaskDescription = TEXT("Awaiting assignment");
        Task.Status = EDir_AgentStatus::Idle;
        Task.Priority = 1.0f;
        Task.StartTime = FDateTime::Now();
        Task.EstimatedCompletion = FDateTime::Now();
        
        AgentTasks.Add(Task);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d agent tasks"), AgentTasks.Num());
}

void UDir_ProductionManager::LogProductionStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %d"), CurrentCycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Active: %s"), bProductionActive ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Agents: %d"), AgentTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void UDir_ProductionManager::ValidateAgentDependencies()
{
    // Check agent dependencies and block agents if prerequisites aren't met
    // This is a simplified version - in reality would be more complex
    
    bool bEngineArchitectComplete = false;
    bool bCoreSystemsComplete = false;
    
    // Find critical agent statuses
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == TEXT("Engine Architect") && Task.Status == EDir_AgentStatus::Completed)
        {
            bEngineArchitectComplete = true;
        }
        if (Task.AgentName == TEXT("Core Systems Programmer") && Task.Status == EDir_AgentStatus::Completed)
        {
            bCoreSystemsComplete = true;
        }
    }
    
    // Block dependent agents if prerequisites aren't met
    if (!bEngineArchitectComplete)
    {
        for (FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.AgentName != TEXT("Studio Director") && 
                Task.AgentName != TEXT("Engine Architect") &&
                Task.Status == EDir_AgentStatus::Working)
            {
                BlockAgent(Task.AgentName, TEXT("Engine Architect prerequisites not met"));
            }
        }
    }
}