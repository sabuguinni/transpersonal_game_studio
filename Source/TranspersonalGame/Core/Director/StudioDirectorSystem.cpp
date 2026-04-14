#include "StudioDirectorSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Initialize production settings
    CycleTimeoutHours = 24.0f;
    bAutoAdvanceAgents = true;
    bStrictSequenceMode = true;
    
    // Initialize state
    CurrentActiveAgent = 1; // Start with Studio Director (self)
    bCycleInProgress = false;
    CreativeVision = TEXT("Create an immersive prehistoric survival experience that connects players with ancient wisdom and transpersonal consciousness.");
    
    InitializeAgentChain();
}

void UStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized - Production Chain Ready"));
    
    // Auto-start first cycle if in development mode
    if (GetWorld() && GetWorld()->IsPlayInEditor())
    {
        StartNewCycle(TEXT("PROD_CYCLE_011"), TEXT("Initial production cycle - establishing base systems"));
    }
}

void UStudioDirectorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bCycleInProgress)
    {
        // Check for cycle timeout
        FDateTime Now = FDateTime::Now();
        FTimespan ElapsedTime = Now - CurrentCycle.StartTime;
        
        if (ElapsedTime.GetTotalHours() > CycleTimeoutHours)
        {
            UE_LOG(LogTemp, Error, TEXT("Production cycle timeout! Cycle: %s"), *CurrentCycle.CycleID);
            BlockCycle(TEXT("Cycle timeout exceeded"));
        }
        
        // Auto-advance agents if enabled
        if (bAutoAdvanceAgents)
        {
            // Check if current agent is ready to advance
            if (IsAgentReadyToAdvance(CurrentActiveAgent))
            {
                AdvanceToNextAgent();
            }
        }
    }
}

void UStudioDirectorSystem::StartNewCycle(const FString& CycleID, const FString& CreativeNotes)
{
    if (bCycleInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start new cycle - current cycle still in progress"));
        return;
    }
    
    // Initialize new cycle
    CurrentCycle.CycleID = CycleID;
    CurrentCycle.CycleNumber++;
    CurrentCycle.StartTime = FDateTime::Now();
    CurrentCycle.EstimatedEndTime = CurrentCycle.StartTime + FTimespan::FromHours(CycleTimeoutHours);
    CurrentCycle.CreativeDirectorNotes = CreativeNotes;
    CurrentCycle.OverallState = EDir_ProductionState::InProgress;
    
    // Reset agent statuses
    CurrentCycle.AgentStatuses.Empty();
    for (const FString& AgentName : AgentChain)
    {
        FDir_AgentStatus NewStatus;
        NewStatus.AgentName = AgentName;
        NewStatus.AgentID = CurrentCycle.AgentStatuses.Num() + 1;
        NewStatus.CurrentState = (NewStatus.AgentID == 1) ? EDir_ProductionState::InProgress : EDir_ProductionState::Planning;
        NewStatus.CurrentTask = (NewStatus.AgentID == 1) ? TEXT("Coordinating production cycle") : TEXT("Awaiting activation");
        CurrentCycle.AgentStatuses.Add(NewStatus);
    }
    
    CurrentActiveAgent = 1;
    bCycleInProgress = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Started production cycle: %s"), *CycleID);
    UE_LOG(LogTemp, Warning, TEXT("Creative Notes: %s"), *CreativeNotes);
}

void UStudioDirectorSystem::CompleteCycle()
{
    if (!bCycleInProgress)
    {
        return;
    }
    
    CurrentCycle.OverallState = EDir_ProductionState::Complete;
    bCycleInProgress = false;
    
    // Log completion statistics
    FDateTime Now = FDateTime::Now();
    FTimespan CycleDuration = Now - CurrentCycle.StartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Production cycle completed: %s"), *CurrentCycle.CycleID);
    UE_LOG(LogTemp, Warning, TEXT("Duration: %.2f hours"), CycleDuration.GetTotalHours());
    UE_LOG(LogTemp, Warning, TEXT("Agents processed: %d"), CurrentCycle.AgentStatuses.Num());
}

void UStudioDirectorSystem::BlockCycle(const FString& Reason)
{
    CurrentCycle.OverallState = EDir_ProductionState::Blocked;
    bCycleInProgress = false;
    
    UE_LOG(LogTemp, Error, TEXT("Production cycle BLOCKED: %s"), *CurrentCycle.CycleID);
    UE_LOG(LogTemp, Error, TEXT("Reason: %s"), *Reason);
}

void UStudioDirectorSystem::RegisterAgent(int32 AgentID, const FString& AgentName)
{
    FDir_AgentStatus NewAgent;
    NewAgent.AgentID = AgentID;
    NewAgent.AgentName = AgentName;
    NewAgent.CurrentState = EDir_ProductionState::Planning;
    NewAgent.CurrentTask = TEXT("Registered");
    
    RegisteredAgents.Add(NewAgent);
    
    UE_LOG(LogTemp, Log, TEXT("Registered agent: %s (ID: %d)"), *AgentName, AgentID);
}

void UStudioDirectorSystem::UpdateAgentStatus(int32 AgentID, EDir_ProductionState NewState, const FString& CurrentTask, float Progress)
{
    // Update in current cycle
    for (FDir_AgentStatus& Status : CurrentCycle.AgentStatuses)
    {
        if (Status.AgentID == AgentID)
        {
            Status.CurrentState = NewState;
            Status.CurrentTask = CurrentTask;
            Status.ProgressPercentage = Progress;
            Status.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Log, TEXT("Agent %s updated: %s (%.1f%%)"), *Status.AgentName, *CurrentTask, Progress);
            return;
        }
    }
    
    // Update in registered agents
    for (FDir_AgentStatus& Status : RegisteredAgents)
    {
        if (Status.AgentID == AgentID)
        {
            Status.CurrentState = NewState;
            Status.CurrentTask = CurrentTask;
            Status.ProgressPercentage = Progress;
            Status.LastUpdate = FDateTime::Now();
            return;
        }
    }
}

FDir_AgentStatus UStudioDirectorSystem::GetAgentStatus(int32 AgentID)
{
    // Check current cycle first
    for (const FDir_AgentStatus& Status : CurrentCycle.AgentStatuses)
    {
        if (Status.AgentID == AgentID)
        {
            return Status;
        }
    }
    
    // Check registered agents
    for (const FDir_AgentStatus& Status : RegisteredAgents)
    {
        if (Status.AgentID == AgentID)
        {
            return Status;
        }
    }
    
    // Return default if not found
    return FDir_AgentStatus();
}

void UStudioDirectorSystem::AdvanceToNextAgent()
{
    if (!bCycleInProgress)
    {
        return;
    }
    
    // Mark current agent as complete
    UpdateAgentStatus(CurrentActiveAgent, EDir_ProductionState::Complete, TEXT("Work completed"), 100.0f);
    
    // Advance to next agent
    CurrentActiveAgent++;
    
    if (CurrentActiveAgent > CurrentCycle.AgentStatuses.Num())
    {
        // All agents completed - finish cycle
        CompleteCycle();
        return;
    }
    
    // Activate next agent
    UpdateAgentStatus(CurrentActiveAgent, EDir_ProductionState::InProgress, TEXT("Starting work"), 0.0f);
    NotifyAgentActivation(CurrentActiveAgent);
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced to agent %d: %s"), CurrentActiveAgent, *GetAgentStatus(CurrentActiveAgent).AgentName);
}

bool UStudioDirectorSystem::ValidateProductionChain()
{
    if (AgentChain.Num() != 19)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid agent chain - expected 19 agents, found %d"), AgentChain.Num());
        return false;
    }
    
    // Validate agent sequence
    if (bStrictSequenceMode)
    {
        for (int32 i = 1; i < CurrentActiveAgent; i++)
        {
            FDir_AgentStatus Status = GetAgentStatus(i);
            if (Status.CurrentState != EDir_ProductionState::Complete)
            {
                UE_LOG(LogTemp, Error, TEXT("Agent %d not complete - cannot advance"), i);
                return false;
            }
        }
    }
    
    return true;
}

TArray<FString> UStudioDirectorSystem::GetProductionWarnings()
{
    TArray<FString> Warnings;
    
    if (!bCycleInProgress)
    {
        Warnings.Add(TEXT("No active production cycle"));
    }
    
    // Check for stalled agents
    FDateTime Now = FDateTime::Now();
    for (const FDir_AgentStatus& Status : CurrentCycle.AgentStatuses)
    {
        if (Status.CurrentState == EDir_ProductionState::InProgress)
        {
            FTimespan TimeSinceUpdate = Now - Status.LastUpdate;
            if (TimeSinceUpdate.GetTotalHours() > 2.0f)
            {
                Warnings.Add(FString::Printf(TEXT("Agent %s stalled for %.1f hours"), *Status.AgentName, TimeSinceUpdate.GetTotalHours()));
            }
        }
    }
    
    return Warnings;
}

float UStudioDirectorSystem::GetOverallProgress()
{
    if (CurrentCycle.AgentStatuses.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_AgentStatus& Status : CurrentCycle.AgentStatuses)
    {
        TotalProgress += Status.ProgressPercentage;
    }
    
    return TotalProgress / CurrentCycle.AgentStatuses.Num();
}

void UStudioDirectorSystem::SetCreativeVision(const FString& VisionStatement)
{
    CreativeVision = VisionStatement;
    UE_LOG(LogTemp, Warning, TEXT("Creative Vision Updated: %s"), *VisionStatement);
}

FString UStudioDirectorSystem::GetCreativeVision()
{
    return CreativeVision;
}

bool UStudioDirectorSystem::ValidateAgainstVision(const FString& ProposedChange)
{
    // Simple keyword validation - in production this would be more sophisticated
    if (CreativeVision.Contains(TEXT("prehistoric")) && !ProposedChange.Contains(TEXT("prehistoric")))
    {
        UE_LOG(LogTemp, Warning, TEXT("Proposed change may not align with prehistoric vision"));
        return false;
    }
    
    return true;
}

void UStudioDirectorSystem::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s"), *CurrentCycle.CycleID);
    UE_LOG(LogTemp, Warning, TEXT("State: %s"), CurrentCycle.OverallState == EDir_ProductionState::InProgress ? TEXT("In Progress") : TEXT("Other"));
    UE_LOG(LogTemp, Warning, TEXT("Active Agent: %d"), CurrentActiveAgent);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), GetOverallProgress());
    
    for (const FDir_AgentStatus& Status : CurrentCycle.AgentStatuses)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent %d (%s): %s - %.1f%%"), Status.AgentID, *Status.AgentName, *Status.CurrentTask, Status.ProgressPercentage);
    }
}

void UStudioDirectorSystem::ResetProductionChain()
{
    bCycleInProgress = false;
    CurrentActiveAgent = 1;
    CurrentCycle = FDir_ProductionCycle();
    RegisteredAgents.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Production chain reset"));
}

void UStudioDirectorSystem::SimulateAgentProgress()
{
    if (!bCycleInProgress)
    {
        StartNewCycle(TEXT("SIMULATION_CYCLE"), TEXT("Simulated production cycle for testing"));
    }
    
    // Simulate progress for current agent
    FDir_AgentStatus CurrentStatus = GetAgentStatus(CurrentActiveAgent);
    float NewProgress = FMath::Min(100.0f, CurrentStatus.ProgressPercentage + 25.0f);
    
    UpdateAgentStatus(CurrentActiveAgent, EDir_ProductionState::InProgress, TEXT("Simulated work"), NewProgress);
    
    if (NewProgress >= 100.0f)
    {
        AdvanceToNextAgent();
    }
}

void UStudioDirectorSystem::InitializeAgentChain()
{
    AgentChain.Empty();
    AgentChain.Add(TEXT("Studio Director"));
    AgentChain.Add(TEXT("Engine Architect"));
    AgentChain.Add(TEXT("Core Systems Programmer"));
    AgentChain.Add(TEXT("Performance Optimizer"));
    AgentChain.Add(TEXT("Procedural World Generator"));
    AgentChain.Add(TEXT("Environment Artist"));
    AgentChain.Add(TEXT("Architecture & Interior Agent"));
    AgentChain.Add(TEXT("Lighting & Atmosphere Agent"));
    AgentChain.Add(TEXT("Character Artist Agent"));
    AgentChain.Add(TEXT("Animation Agent"));
    AgentChain.Add(TEXT("NPC Behavior Agent"));
    AgentChain.Add(TEXT("Combat & Enemy AI Agent"));
    AgentChain.Add(TEXT("Crowd & Traffic Simulation"));
    AgentChain.Add(TEXT("Quest & Mission Designer"));
    AgentChain.Add(TEXT("Narrative & Dialogue Agent"));
    AgentChain.Add(TEXT("Audio Agent"));
    AgentChain.Add(TEXT("VFX Agent"));
    AgentChain.Add(TEXT("QA & Testing Agent"));
    AgentChain.Add(TEXT("Integration & Build Agent"));
}

void UStudioDirectorSystem::ValidateAgentSequence()
{
    // Ensure proper dependencies are met
    // This would contain complex validation logic in production
}

bool UStudioDirectorSystem::IsAgentReadyToAdvance(int32 AgentID)
{
    FDir_AgentStatus Status = GetAgentStatus(AgentID);
    return Status.ProgressPercentage >= 100.0f && Status.CurrentState == EDir_ProductionState::InProgress;
}

void UStudioDirectorSystem::NotifyAgentActivation(int32 AgentID)
{
    FDir_AgentStatus Status = GetAgentStatus(AgentID);
    UE_LOG(LogTemp, Warning, TEXT("AGENT ACTIVATION: %s (ID: %d) - Begin Work"), *Status.AgentName, AgentID);
    
    // In production, this would trigger the actual agent via API
}