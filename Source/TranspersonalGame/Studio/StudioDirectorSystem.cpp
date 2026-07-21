#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/TranspersonalGame.h"

void UStudioDirectorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Studio Director System initializing..."));
    
    InitializeAgents();
    bSystemInitialized = true;
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Studio Director System initialized with %d agents"), Agents.Num());
}

void UStudioDirectorSystem::Deinitialize()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Studio Director System shutting down..."));
    
    if (CurrentCycle.bCycleCompleted == false && !CurrentCycle.CycleID.IsEmpty())
    {
        CompleteProductionCycle();
    }
    
    Agents.Empty();
    CycleHistory.Empty();
    bSystemInitialized = false;
    
    Super::Deinitialize();
}

void UStudioDirectorSystem::InitializeAgents()
{
    Agents.Empty();
    SetupAgentDefinitions();
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Initialized %d production agents"), Agents.Num());
}

void UStudioDirectorSystem::SetupAgentDefinitions()
{
    // Define all 19 agents in the production pipeline
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),           // #01
        TEXT("Engine Architect"),          // #02
        TEXT("Core Systems Programmer"),   // #03
        TEXT("Performance Optimizer"),     // #04
        TEXT("Procedural World Generator"), // #05
        TEXT("Environment Artist"),        // #06
        TEXT("Architecture & Interior"),   // #07
        TEXT("Lighting & Atmosphere"),     // #08
        TEXT("Character Artist"),          // #09
        TEXT("Animation Agent"),           // #10
        TEXT("NPC Behavior Agent"),        // #11
        TEXT("Combat & Enemy AI"),         // #12
        TEXT("Crowd & Traffic Simulation"), // #13
        TEXT("Quest & Mission Designer"),  // #14
        TEXT("Narrative & Dialogue"),      // #15
        TEXT("Audio Agent"),               // #16
        TEXT("VFX Agent"),                 // #17
        TEXT("QA & Testing"),              // #18
        TEXT("Integration & Build")        // #19
    };
    
    for (int32 i = 0; i < AgentNames.Num(); ++i)
    {
        FDir_AgentInfo NewAgent;
        NewAgent.AgentID = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CurrentTask = TEXT("Awaiting assignment");
        NewAgent.ProgressPercentage = 0.0f;
        NewAgent.LastOutput = TEXT("System initialized");
        
        Agents.Add(NewAgent);
    }
}

void UStudioDirectorSystem::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    if (AgentID < 1 || AgentID > Agents.Num())
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Invalid AgentID: %d"), AgentID);
        return;
    }
    
    FDir_AgentInfo& Agent = Agents[AgentID - 1];
    Agent.Status = NewStatus;
    Agent.CurrentTask = TaskDescription;
    
    // Update progress based on status
    switch (NewStatus)
    {
        case EDir_AgentStatus::Idle:
            Agent.ProgressPercentage = 0.0f;
            break;
        case EDir_AgentStatus::Working:
            Agent.ProgressPercentage = 50.0f;
            break;
        case EDir_AgentStatus::Completed:
            Agent.ProgressPercentage = 100.0f;
            break;
        case EDir_AgentStatus::Error:
        case EDir_AgentStatus::Blocked:
            // Keep current progress
            break;
    }
    
    LogAgentActivity(Agent);
    OnAgentStatusChanged.Broadcast(Agent);
}

FDir_AgentInfo UStudioDirectorSystem::GetAgentInfo(int32 AgentID) const
{
    if (AgentID >= 1 && AgentID <= Agents.Num())
    {
        return Agents[AgentID - 1];
    }
    
    return FDir_AgentInfo();
}

TArray<FDir_AgentInfo> UStudioDirectorSystem::GetAllAgents() const
{
    return Agents;
}

void UStudioDirectorSystem::StartProductionCycle(const FString& CycleID)
{
    // Complete previous cycle if still active
    if (!CurrentCycle.CycleID.IsEmpty() && !CurrentCycle.bCycleCompleted)
    {
        CompleteProductionCycle();
    }
    
    // Start new cycle
    CurrentCycle = FDir_ProductionCycle();
    CurrentCycle.CycleID = CycleID;
    CurrentCycle.StartTime = FDateTime::Now();
    CurrentCycle.bCycleCompleted = false;
    
    // Reset all agents to idle
    for (FDir_AgentInfo& Agent : Agents)
    {
        Agent.Status = EDir_AgentStatus::Idle;
        Agent.CurrentTask = TEXT("Cycle starting");
        Agent.ProgressPercentage = 0.0f;
    }
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Started production cycle: %s"), *CycleID);
}

void UStudioDirectorSystem::CompleteProductionCycle()
{
    if (CurrentCycle.CycleID.IsEmpty())
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("No active cycle to complete"));
        return;
    }
    
    CurrentCycle.EndTime = FDateTime::Now();
    CurrentCycle.bCycleCompleted = true;
    CurrentCycle.AgentResults = Agents;
    
    // Count completed agents
    int32 CompletedAgents = 0;
    for (const FDir_AgentInfo& Agent : Agents)
    {
        if (Agent.Status == EDir_AgentStatus::Completed)
        {
            CompletedAgents++;
        }
    }
    
    // Add to history
    CycleHistory.Add(CurrentCycle);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Completed production cycle: %s (%d/%d agents completed)"), 
           *CurrentCycle.CycleID, CompletedAgents, Agents.Num());
    
    OnCycleCompleted.Broadcast(CurrentCycle);
    
    // Clear current cycle
    CurrentCycle = FDir_ProductionCycle();
}

FDir_ProductionCycle UStudioDirectorSystem::GetCurrentCycle() const
{
    return CurrentCycle;
}

TArray<FDir_ProductionCycle> UStudioDirectorSystem::GetCycleHistory() const
{
    return CycleHistory;
}

bool UStudioDirectorSystem::ValidateMinPlayableMap()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTranspersonalGame, Error, TEXT("No valid world for map validation"));
        return false;
    }
    
    bool bValidationPassed = true;
    
    // Check for essential actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    bool bHasTerrain = false;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetName().Contains(TEXT("PlayerStart")))
        {
            bHasPlayerStart = true;
        }
        else if (Actor->GetName().Contains(TEXT("DirectionalLight")) || Actor->GetName().Contains(TEXT("Sun")))
        {
            bHasLighting = true;
        }
        else if (Actor->GetName().Contains(TEXT("Landscape")) || Actor->GetName().Contains(TEXT("Terrain")))
        {
            bHasTerrain = true;
        }
    }
    
    if (!bHasPlayerStart)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Map validation failed: No PlayerStart found"));
        bValidationPassed = false;
    }
    
    if (!bHasLighting)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Map validation failed: No lighting found"));
        bValidationPassed = false;
    }
    
    if (!bHasTerrain)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Map validation failed: No terrain found"));
        bValidationPassed = false;
    }
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Map validation %s"), bValidationPassed ? TEXT("PASSED") : TEXT("FAILED"));
    return bValidationPassed;
}

void UStudioDirectorSystem::GenerateProductionReport()
{
    if (!bSystemInitialized)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot generate report - system not initialized"));
        return;
    }
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("=== PRODUCTION REPORT ==="));
    UE_LOG(LogTranspersonalGame, Log, TEXT("Current Cycle: %s"), *CurrentCycle.CycleID);
    UE_LOG(LogTranspersonalGame, Log, TEXT("Total Cycles Completed: %d"), CycleHistory.Num());
    
    int32 IdleAgents = 0;
    int32 WorkingAgents = 0;
    int32 CompletedAgents = 0;
    int32 ErrorAgents = 0;
    int32 BlockedAgents = 0;
    
    for (const FDir_AgentInfo& Agent : Agents)
    {
        switch (Agent.Status)
        {
            case EDir_AgentStatus::Idle: IdleAgents++; break;
            case EDir_AgentStatus::Working: WorkingAgents++; break;
            case EDir_AgentStatus::Completed: CompletedAgents++; break;
            case EDir_AgentStatus::Error: ErrorAgents++; break;
            case EDir_AgentStatus::Blocked: BlockedAgents++; break;
        }
    }
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Agent Status: %d Idle, %d Working, %d Completed, %d Error, %d Blocked"), 
           IdleAgents, WorkingAgents, CompletedAgents, ErrorAgents, BlockedAgents);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("=== END REPORT ==="));
}

void UStudioDirectorSystem::LogAgentActivity(const FDir_AgentInfo& AgentInfo)
{
    FString StatusString;
    switch (AgentInfo.Status)
    {
        case EDir_AgentStatus::Idle: StatusString = TEXT("IDLE"); break;
        case EDir_AgentStatus::Working: StatusString = TEXT("WORKING"); break;
        case EDir_AgentStatus::Completed: StatusString = TEXT("COMPLETED"); break;
        case EDir_AgentStatus::Error: StatusString = TEXT("ERROR"); break;
        case EDir_AgentStatus::Blocked: StatusString = TEXT("BLOCKED"); break;
    }
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Agent #%02d (%s): %s - %s [%.1f%%]"), 
           AgentInfo.AgentID, *AgentInfo.AgentName, *StatusString, *AgentInfo.CurrentTask, AgentInfo.ProgressPercentage);
}

bool UStudioDirectorSystem::CheckAgentDependencies(int32 AgentID)
{
    // Simple dependency check - previous agents should be completed or working
    if (AgentID <= 1)
    {
        return true; // Studio Director has no dependencies
    }
    
    for (int32 i = 0; i < AgentID - 1; ++i)
    {
        const FDir_AgentInfo& PreviousAgent = Agents[i];
        if (PreviousAgent.Status == EDir_AgentStatus::Error || PreviousAgent.Status == EDir_AgentStatus::Blocked)
        {
            return false;
        }
    }
    
    return true;
}