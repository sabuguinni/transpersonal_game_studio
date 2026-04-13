#include "StudioDirectorSubsystem.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"

void UStudioDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Subsystem Initialized"));
    
    InitializeAgentStates();
    Metrics = FDir_ProductionMetrics();
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    bProductionInProgress = false;
    CurrentActiveAgent = 1;
    
    LogProductionEvent(TEXT("System Initialized"), TEXT("Studio Director Subsystem ready for production"));
}

void UStudioDirectorSubsystem::Deinitialize()
{
    if (bProductionInProgress)
    {
        LogProductionEvent(TEXT("Emergency Shutdown"), TEXT("Production was in progress during shutdown"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Subsystem Deinitialized"));
    Super::Deinitialize();
}

void UStudioDirectorSubsystem::StartProductionCycle(const FString& CycleID)
{
    if (bProductionInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start new cycle - production already in progress"));
        return;
    }
    
    CurrentCycleID = CycleID;
    bProductionInProgress = true;
    CurrentPhase = EDir_ProductionPhase::Production;
    CurrentActiveAgent = 1;
    
    // Reset all agents to idle
    for (FDir_AgentInfo& Agent : AgentStates)
    {
        Agent.Status = EDir_AgentStatus::Idle;
        Agent.Progress = 0.0f;
        Agent.CurrentTask = TEXT("Awaiting activation");
        Agent.LastUpdate = FDateTime::Now();
    }
    
    // Activate Studio Director (Agent #1)
    if (AgentStates.IsValidIndex(0))
    {
        AgentStates[0].Status = EDir_AgentStatus::Working;
        AgentStates[0].CurrentTask = TEXT("Coordinating production cycle");
        AgentStates[0].Progress = 0.0f;
    }
    
    Metrics.CycleStartTime = FDateTime::Now();
    UpdateMetrics();
    
    LogProductionEvent(TEXT("Production Cycle Started"), FString::Printf(TEXT("Cycle ID: %s"), *CycleID));
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle Started: %s"), *CycleID);
}

void UStudioDirectorSubsystem::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription, float Progress)
{
    int32 AgentIndex = AgentID - 1; // Convert to 0-based index
    
    if (!AgentStates.IsValidIndex(AgentIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Agent ID: %d"), AgentID);
        return;
    }
    
    FDir_AgentInfo& Agent = AgentStates[AgentIndex];
    Agent.Status = NewStatus;
    Agent.CurrentTask = TaskDescription;
    Agent.Progress = FMath::Clamp(Progress, 0.0f, 100.0f);
    Agent.LastUpdate = FDateTime::Now();
    
    UpdateMetrics();
    
    LogProductionEvent(
        FString::Printf(TEXT("Agent %d Status Update"), AgentID),
        FString::Printf(TEXT("Status: %s, Task: %s, Progress: %.1f%%"), 
            *UEnum::GetValueAsString(NewStatus), *TaskDescription, Progress)
    );
}

void UStudioDirectorSubsystem::CompleteAgentTask(int32 AgentID, const FString& Output)
{
    UpdateAgentStatus(AgentID, EDir_AgentStatus::Complete, TEXT("Task completed"), 100.0f);
    
    // If this agent is complete, consider triggering the next one
    if (AgentID == CurrentActiveAgent)
    {
        TriggerNextAgent();
    }
    
    LogProductionEvent(
        FString::Printf(TEXT("Agent %d Task Complete"), AgentID),
        FString::Printf(TEXT("Output: %s"), *Output)
    );
}

void UStudioDirectorSubsystem::BlockAgent(int32 AgentID, const FString& BlockReason)
{
    UpdateAgentStatus(AgentID, EDir_AgentStatus::Blocked, 
        FString::Printf(TEXT("BLOCKED: %s"), *BlockReason), 0.0f);
    
    LogProductionEvent(
        FString::Printf(TEXT("Agent %d Blocked"), AgentID),
        FString::Printf(TEXT("Reason: %s"), *BlockReason)
    );
    
    // If the current active agent is blocked, this might require intervention
    if (AgentID == CurrentActiveAgent)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Active agent %d is blocked - production halted"), AgentID);
    }
}

FDir_ProductionMetrics UStudioDirectorSubsystem::GetProductionMetrics() const
{
    return Metrics;
}

TArray<FDir_AgentInfo> UStudioDirectorSubsystem::GetAllAgentStates() const
{
    return AgentStates;
}

FDir_AgentInfo UStudioDirectorSubsystem::GetAgentInfo(int32 AgentID) const
{
    int32 AgentIndex = AgentID - 1;
    
    if (AgentStates.IsValidIndex(AgentIndex))
    {
        return AgentStates[AgentIndex];
    }
    
    return FDir_AgentInfo(); // Return default if invalid
}

bool UStudioDirectorSubsystem::CanAgentProceed(int32 AgentID) const
{
    // Basic dependency check - agents must proceed in order
    if (AgentID <= CurrentActiveAgent)
    {
        return true;
    }
    
    // Check if previous agent is complete
    int32 PreviousAgentIndex = AgentID - 2;
    if (AgentStates.IsValidIndex(PreviousAgentIndex))
    {
        return AgentStates[PreviousAgentIndex].Status == EDir_AgentStatus::Complete;
    }
    
    return false;
}

void UStudioDirectorSubsystem::TriggerNextAgent()
{
    if (CurrentActiveAgent >= 19)
    {
        // All agents complete - cycle finished
        bProductionInProgress = false;
        CurrentPhase = EDir_ProductionPhase::Release;
        
        LogProductionEvent(TEXT("Production Cycle Complete"), 
            FString::Printf(TEXT("Cycle %s completed successfully"), *CurrentCycleID));
        
        UE_LOG(LogTemp, Warning, TEXT("Production Cycle Complete: %s"), *CurrentCycleID);
        return;
    }
    
    CurrentActiveAgent++;
    
    int32 NextAgentIndex = CurrentActiveAgent - 1;
    if (AgentStates.IsValidIndex(NextAgentIndex))
    {
        AgentStates[NextAgentIndex].Status = EDir_AgentStatus::Working;
        AgentStates[NextAgentIndex].CurrentTask = TEXT("Agent activated - beginning work");
        AgentStates[NextAgentIndex].Progress = 0.0f;
        AgentStates[NextAgentIndex].LastUpdate = FDateTime::Now();
    }
    
    UpdateMetrics();
    
    LogProductionEvent(
        FString::Printf(TEXT("Agent %d Activated"), CurrentActiveAgent),
        TEXT("Next agent in pipeline activated")
    );
}

void UStudioDirectorSubsystem::EmergencyStop(const FString& Reason)
{
    bProductionInProgress = false;
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    
    // Set all working agents to error state
    for (FDir_AgentInfo& Agent : AgentStates)
    {
        if (Agent.Status == EDir_AgentStatus::Working)
        {
            Agent.Status = EDir_AgentStatus::Error;
            Agent.CurrentTask = FString::Printf(TEXT("EMERGENCY STOP: %s"), *Reason);
        }
    }
    
    UpdateMetrics();
    
    LogProductionEvent(TEXT("EMERGENCY STOP"), Reason);
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY STOP: %s"), *Reason);
}

void UStudioDirectorSubsystem::ResetProductionPipeline()
{
    bProductionInProgress = false;
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    CurrentActiveAgent = 1;
    CurrentCycleID = TEXT("");
    
    InitializeAgentStates();
    Metrics = FDir_ProductionMetrics();
    
    LogProductionEvent(TEXT("Pipeline Reset"), TEXT("All agents reset to initial state"));
    UE_LOG(LogTemp, Warning, TEXT("Production Pipeline Reset"));
}

void UStudioDirectorSubsystem::DebugLogAllAgentStates()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT STATE DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Production In Progress: %s"), bProductionInProgress ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Current Active Agent: %d"), CurrentActiveAgent);
    UE_LOG(LogTemp, Warning, TEXT("Current Cycle: %s"), *CurrentCycleID);
    
    for (int32 i = 0; i < AgentStates.Num(); i++)
    {
        const FDir_AgentInfo& Agent = AgentStates[i];
        UE_LOG(LogTemp, Warning, TEXT("Agent %d (%s): %s - %s (%.1f%%)"), 
            Agent.AgentID, *Agent.AgentName, 
            *UEnum::GetValueAsString(Agent.Status),
            *Agent.CurrentTask, Agent.Progress);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END AGENT STATE DEBUG ==="));
}

void UStudioDirectorSubsystem::SimulateProductionCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("Simulating Production Cycle..."));
    
    StartProductionCycle(TEXT("SIMULATION_001"));
    
    // Simulate some agent progress
    for (int32 i = 1; i <= 5; i++)
    {
        UpdateAgentStatus(i, EDir_AgentStatus::Working, 
            FString::Printf(TEXT("Simulated task for agent %d"), i), 50.0f);
        
        FPlatformProcess::Sleep(0.1f); // Small delay for simulation
        
        CompleteAgentTask(i, FString::Printf(TEXT("Simulated output from agent %d"), i));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle Simulation Complete"));
}

void UStudioDirectorSubsystem::InitializeAgentStates()
{
    AgentStates.Empty();
    
    // Define all 19 agents
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
    
    for (int32 i = 0; i < 19; i++)
    {
        FDir_AgentInfo NewAgent;
        NewAgent.AgentID = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CurrentTask = TEXT("Awaiting activation");
        NewAgent.Progress = 0.0f;
        NewAgent.LastUpdate = FDateTime::Now();
        
        AgentStates.Add(NewAgent);
    }
}

void UStudioDirectorSubsystem::UpdateMetrics()
{
    Metrics.LastUpdateTime = FDateTime::Now();
    
    Metrics.ActiveAgents = 0;
    Metrics.CompletedTasks = 0;
    Metrics.BlockedAgents = 0;
    
    float TotalProgress = 0.0f;
    
    for (const FDir_AgentInfo& Agent : AgentStates)
    {
        switch (Agent.Status)
        {
            case EDir_AgentStatus::Working:
                Metrics.ActiveAgents++;
                break;
            case EDir_AgentStatus::Complete:
                Metrics.CompletedTasks++;
                break;
            case EDir_AgentStatus::Blocked:
            case EDir_AgentStatus::Error:
                Metrics.BlockedAgents++;
                break;
        }
        
        TotalProgress += Agent.Progress;
    }
    
    Metrics.OverallProgress = TotalProgress / (AgentStates.Num() * 100.0f) * 100.0f;
}

bool UStudioDirectorSubsystem::ValidateAgentDependencies(int32 AgentID) const
{
    // Implement specific dependency validation logic here
    // For now, simple sequential validation
    return CanAgentProceed(AgentID);
}

void UStudioDirectorSubsystem::LogProductionEvent(const FString& Event, const FString& Details)
{
    FString LogMessage = FString::Printf(TEXT("[STUDIO DIRECTOR] %s"), *Event);
    if (!Details.IsEmpty())
    {
        LogMessage += FString::Printf(TEXT(" - %s"), *Details);
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
    
    // In a real implementation, you might want to save this to a file or database
}