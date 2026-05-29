#include "DirectorProductionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Engine/GameInstance.h"

UDirectorProductionManager::UDirectorProductionManager()
{
    MaxCycleHistory = 50;
    CriticalIssueThreshold = 3.0f;
    bAutoGenerateReports = true;
}

void UDirectorProductionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Initializing Studio Director production system"));
    
    // Initialize the 19-agent chain
    InitializeAgentChain();
    
    // Load previous production state if available
    LoadProductionState();
    
    // Start with a fresh cycle if none exists
    if (CurrentCycle.CycleID.IsEmpty())
    {
        StartNewProductionCycle(TEXT("PROD_CYCLE_017"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Studio Director system ready"));
}

void UDirectorProductionManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Shutting down Studio Director system"));
    
    // Save current production state
    SaveProductionState();
    
    // Complete current cycle if active
    if (!CurrentCycle.bCycleCompleted)
    {
        CompleteCurrentCycle();
    }
    
    Super::Deinitialize();
}

void UDirectorProductionManager::StartNewProductionCycle(const FString& CycleID)
{
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Starting new production cycle: %s"), *CycleID);
    
    // Complete previous cycle if active
    if (!CurrentCycle.bCycleCompleted && !CurrentCycle.CycleID.IsEmpty())
    {
        CompleteCurrentCycle();
    }
    
    // Initialize new cycle
    CurrentCycle = FDir_ProductionCycle();
    CurrentCycle.CycleID = CycleID;
    CurrentCycle.CycleNumber = CycleHistory.Num() + 1;
    CurrentCycle.StartTime = FDateTime::Now();
    CurrentCycle.bCycleCompleted = false;
    
    // Reset agent statuses for new cycle
    for (auto& AgentPair : AgentStatuses)
    {
        AgentPair.Value.bIsActive = false;
        AgentPair.Value.CurrentTask = TEXT("Awaiting cycle start");
        AgentPair.Value.TaskProgress = 0.0f;
        AgentPair.Value.LastUpdate = FDateTime::Now();
    }
    
    // Activate Studio Director (Agent #01)
    if (AgentStatuses.Contains(TEXT("01")))
    {
        AgentStatuses[TEXT("01")].bIsActive = true;
        AgentStatuses[TEXT("01")].CurrentTask = TEXT("Cycle coordination and task distribution");
        AgentStatuses[TEXT("01")].TaskProgress = 10.0f;
    }
    
    UpdateMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Cycle %s started successfully"), *CycleID);
}

void UDirectorProductionManager::CompleteCurrentCycle()
{
    if (CurrentCycle.bCycleCompleted)
    {
        UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Cycle already completed"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Completing cycle %s"), *CurrentCycle.CycleID);
    
    // Mark cycle as completed
    CurrentCycle.bCycleCompleted = true;
    CurrentCycle.EndTime = FDateTime::Now();
    CurrentCycle.CompletionPercentage = 100.0f;
    
    // Calculate cycle duration
    FTimespan CycleDuration = CurrentCycle.EndTime - CurrentCycle.StartTime;
    
    // Add to history
    CycleHistory.Add(CurrentCycle);
    
    // Maintain history limit
    if (CycleHistory.Num() > MaxCycleHistory)
    {
        CycleHistory.RemoveAt(0);
    }
    
    // Update metrics
    UpdateMetrics();
    
    // Generate report if auto-generation is enabled
    if (bAutoGenerateReports)
    {
        GenerateProductionReport();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Cycle completed in %f minutes"), CycleDuration.GetTotalMinutes());
}

FDir_ProductionCycle UDirectorProductionManager::GetCurrentCycle() const
{
    return CurrentCycle;
}

TArray<FDir_ProductionCycle> UDirectorProductionManager::GetCycleHistory() const
{
    return CycleHistory;
}

void UDirectorProductionManager::RegisterAgent(const FString& AgentID, const FString& AgentName, int32 AgentNumber)
{
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Registering agent %s - %s (#%d)"), *AgentID, *AgentName, AgentNumber);
    
    FDir_AgentStatus NewAgent;
    NewAgent.AgentID = AgentID;
    NewAgent.AgentName = AgentName;
    NewAgent.AgentNumber = AgentNumber;
    NewAgent.bIsActive = false;
    NewAgent.CurrentTask = TEXT("Registered, awaiting activation");
    NewAgent.TaskProgress = 0.0f;
    NewAgent.LastUpdate = FDateTime::Now();
    
    AgentStatuses.Add(AgentID, NewAgent);
}

void UDirectorProductionManager::UpdateAgentStatus(const FString& AgentID, const FString& CurrentTask, float Progress)
{
    if (!AgentStatuses.Contains(AgentID))
    {
        UE_LOG(LogTemp, Error, TEXT("DirectorProductionManager: Agent %s not registered"), *AgentID);
        return;
    }
    
    FDir_AgentStatus& Agent = AgentStatuses[AgentID];
    Agent.CurrentTask = CurrentTask;
    Agent.TaskProgress = FMath::Clamp(Progress, 0.0f, 100.0f);
    Agent.LastUpdate = FDateTime::Now();
    Agent.bIsActive = (Progress > 0.0f && Progress < 100.0f);
    
    // Add to current cycle if task completed
    if (Progress >= 100.0f)
    {
        FString CompletedTask = FString::Printf(TEXT("Agent %s: %s"), *AgentID, *CurrentTask);
        CurrentCycle.CompletedTasks.AddUnique(CompletedTask);
        Agent.CompletedOutputs.Add(CurrentTask);
    }
    
    // Update active agents list
    CurrentCycle.ActiveAgents.RemoveAll([AgentID](const FString& ID) { return ID == AgentID; });
    if (Agent.bIsActive)
    {
        CurrentCycle.ActiveAgents.AddUnique(AgentID);
    }
    
    UpdateMetrics();
}

FDir_AgentStatus UDirectorProductionManager::GetAgentStatus(const FString& AgentID) const
{
    if (AgentStatuses.Contains(AgentID))
    {
        return AgentStatuses[AgentID];
    }
    
    return FDir_AgentStatus();
}

TArray<FDir_AgentStatus> UDirectorProductionManager::GetAllAgentStatuses() const
{
    TArray<FDir_AgentStatus> AllStatuses;
    for (const auto& AgentPair : AgentStatuses)
    {
        AllStatuses.Add(AgentPair.Value);
    }
    
    // Sort by agent number
    AllStatuses.Sort([](const FDir_AgentStatus& A, const FDir_AgentStatus& B) {
        return A.AgentNumber < B.AgentNumber;
    });
    
    return AllStatuses;
}

FDir_ProductionMetrics UDirectorProductionManager::GetProductionMetrics() const
{
    return Metrics;
}

void UDirectorProductionManager::RecordFileCreation(const FString& FilePath, const FString& AgentID)
{
    UE_LOG(LogTemp, Log, TEXT("DirectorProductionManager: File created by Agent %s: %s"), *AgentID, *FilePath);
    
    Metrics.TotalFilesCreated++;
    
    // Update agent's completed outputs
    if (AgentStatuses.Contains(AgentID))
    {
        FDir_AgentStatus& Agent = AgentStatuses[AgentID];
        Agent.CompletedOutputs.AddUnique(FilePath);
    }
    
    UpdateMetrics();
}

void UDirectorProductionManager::RecordUE5Command(const FString& CommandType, const FString& AgentID)
{
    UE_LOG(LogTemp, Log, TEXT("DirectorProductionManager: UE5 command executed by Agent %s: %s"), *AgentID, *CommandType);
    
    Metrics.TotalUE5Commands++;
    UpdateMetrics();
}

void UDirectorProductionManager::ReportIssue(const FString& IssueDescription, const FString& AgentID, bool bIsCritical)
{
    FString IssueReport = FString::Printf(TEXT("[Agent %s] %s%s"), 
        *AgentID, 
        bIsCritical ? TEXT("CRITICAL: ") : TEXT(""), 
        *IssueDescription);
    
    if (bIsCritical)
    {
        Metrics.CriticalIssues.Add(IssueReport);
        UE_LOG(LogTemp, Error, TEXT("DirectorProductionManager: CRITICAL ISSUE - %s"), *IssueReport);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Issue reported - %s"), *IssueReport);
    }
}

TArray<FString> UDirectorProductionManager::GetCriticalIssues() const
{
    return Metrics.CriticalIssues;
}

void UDirectorProductionManager::ClearResolvedIssues()
{
    int32 PreviousCount = Metrics.CriticalIssues.Num();
    Metrics.CriticalIssues.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Cleared %d resolved issues"), PreviousCount);
}

void UDirectorProductionManager::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s (Number: %d)"), *CurrentCycle.CycleID, CurrentCycle.CycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), CurrentCycle.bCycleCompleted ? TEXT("COMPLETED") : TEXT("ACTIVE"));
    UE_LOG(LogTemp, Warning, TEXT("Progress: %.1f%%"), CurrentCycle.CompletionPercentage);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), CurrentCycle.ActiveAgents.Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), CurrentCycle.CompletedTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Total Files Created: %d"), Metrics.TotalFilesCreated);
    UE_LOG(LogTemp, Warning, TEXT("Total UE5 Commands: %d"), Metrics.TotalUE5Commands);
    UE_LOG(LogTemp, Warning, TEXT("Critical Issues: %d"), Metrics.CriticalIssues.Num());
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), Metrics.OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
}

bool UDirectorProductionManager::ShouldBlockProduction() const
{
    // Block production if too many critical issues
    return Metrics.CriticalIssues.Num() >= CriticalIssueThreshold;
}

FString UDirectorProductionManager::GetNextRecommendedAction() const
{
    if (ShouldBlockProduction())
    {
        return TEXT("RESOLVE CRITICAL ISSUES BEFORE PROCEEDING");
    }
    
    if (CurrentCycle.ActiveAgents.Num() == 0 && !CurrentCycle.bCycleCompleted)
    {
        return TEXT("ACTIVATE NEXT AGENT IN CHAIN");
    }
    
    if (CurrentCycle.bCycleCompleted)
    {
        return TEXT("START NEW PRODUCTION CYCLE");
    }
    
    return TEXT("MONITOR ACTIVE AGENTS PROGRESS");
}

void UDirectorProductionManager::InitializeAgentChain()
{
    // Register all 19 agents in the production chain
    RegisterAgent(TEXT("01"), TEXT("Studio Director"), 1);
    RegisterAgent(TEXT("02"), TEXT("Engine Architect"), 2);
    RegisterAgent(TEXT("03"), TEXT("Core Systems Programmer"), 3);
    RegisterAgent(TEXT("04"), TEXT("Performance Optimizer"), 4);
    RegisterAgent(TEXT("05"), TEXT("Procedural World Generator"), 5);
    RegisterAgent(TEXT("06"), TEXT("Environment Artist"), 6);
    RegisterAgent(TEXT("07"), TEXT("Architecture & Interior Agent"), 7);
    RegisterAgent(TEXT("08"), TEXT("Lighting & Atmosphere Agent"), 8);
    RegisterAgent(TEXT("09"), TEXT("Character Artist Agent"), 9);
    RegisterAgent(TEXT("10"), TEXT("Animation Agent"), 10);
    RegisterAgent(TEXT("11"), TEXT("NPC Behavior Agent"), 11);
    RegisterAgent(TEXT("12"), TEXT("Combat & Enemy AI Agent"), 12);
    RegisterAgent(TEXT("13"), TEXT("Crowd & Traffic Simulation"), 13);
    RegisterAgent(TEXT("14"), TEXT("Quest & Mission Designer"), 14);
    RegisterAgent(TEXT("15"), TEXT("Narrative & Dialogue Agent"), 15);
    RegisterAgent(TEXT("16"), TEXT("Audio Agent"), 16);
    RegisterAgent(TEXT("17"), TEXT("VFX Agent"), 17);
    RegisterAgent(TEXT("18"), TEXT("QA & Testing Agent"), 18);
    RegisterAgent(TEXT("19"), TEXT("Integration & Build Agent"), 19);
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorProductionManager: Initialized 19-agent production chain"));
}

void UDirectorProductionManager::UpdateMetrics()
{
    // Calculate overall progress based on completed tasks and active agents
    int32 TotalPossibleTasks = AgentStatuses.Num() * 5; // Assume 5 major tasks per agent
    float CompletedTasksWeight = CurrentCycle.CompletedTasks.Num() / (float)FMath::Max(TotalPossibleTasks, 1);
    
    // Update current cycle progress
    if (CurrentCycle.ActiveAgents.Num() > 0)
    {
        float ActiveProgress = 0.0f;
        for (const FString& AgentID : CurrentCycle.ActiveAgents)
        {
            if (AgentStatuses.Contains(AgentID))
            {
                ActiveProgress += AgentStatuses[AgentID].TaskProgress;
            }
        }
        CurrentCycle.CompletionPercentage = ActiveProgress / CurrentCycle.ActiveAgents.Num();
    }
    
    // Update overall metrics
    Metrics.TotalCycles = CycleHistory.Num() + (CurrentCycle.bCycleCompleted ? 0 : 1);
    Metrics.CompletedCycles = CycleHistory.Num();
    
    // Calculate average cycle time
    if (CycleHistory.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (const FDir_ProductionCycle& Cycle : CycleHistory)
        {
            if (Cycle.bCycleCompleted)
            {
                FTimespan Duration = Cycle.EndTime - Cycle.StartTime;
                TotalTime += Duration.GetTotalMinutes();
            }
        }
        Metrics.AverageCycleTime = TotalTime / CycleHistory.Num();
    }
    
    // Calculate overall project progress
    Metrics.OverallProgress = (Metrics.CompletedCycles / (float)FMath::Max(Metrics.TotalCycles, 1)) * 100.0f;
}

void UDirectorProductionManager::SaveProductionState()
{
    // Implementation for saving production state to disk
    // This would typically save to a JSON file or database
    UE_LOG(LogTemp, Log, TEXT("DirectorProductionManager: Saving production state"));
}

void UDirectorProductionManager::LoadProductionState()
{
    // Implementation for loading production state from disk
    // This would typically load from a JSON file or database
    UE_LOG(LogTemp, Log, TEXT("DirectorProductionManager: Loading production state"));
}