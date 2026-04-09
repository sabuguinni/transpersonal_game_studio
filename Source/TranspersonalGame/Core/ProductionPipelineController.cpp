#include "ProductionPipelineController.h"
#include "StudioDirectorSubsystem.h"
#include "EngineArchitectureValidator.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/DateTime.h"

UProductionPipelineController::UProductionPipelineController()
{
    CurrentAgentIndex = 0;
    bProductionActive = false;
    bEmergencyStop = false;
    StudioDirector = nullptr;
    ArchitectureValidator = nullptr;
}

void UProductionPipelineController::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionPipelineController: Initializing production pipeline"));
    
    // Initialize agent definitions
    InitializeAgentDefinitions();
    
    // Get references to other subsystems
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        StudioDirector = GameInstance->GetSubsystem<UStudioDirectorSubsystem>();
        if (!StudioDirector)
        {
            UE_LOG(LogTemp, Error, TEXT("ProductionPipelineController: Failed to get StudioDirectorSubsystem"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionPipelineController: Initialization complete"));
}

void UProductionPipelineController::Deinitialize()
{
    if (bProductionActive)
    {
        EmergencyStop(TEXT("System shutdown"));
    }
    
    Super::Deinitialize();
}

void UProductionPipelineController::StartProductionCycle(const FString& CycleID, const FString& InitialDirective)
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionPipelineController: Starting production cycle %s"), *CycleID);
    
    CurrentCycleID = CycleID;
    CurrentAgentIndex = 0;
    bProductionActive = true;
    bEmergencyStop = false;
    
    // Reset all agents to idle
    for (FAgentWorkItem& Agent : AgentWorkItems)
    {
        Agent.Status = EAgentStatus::Idle;
        Agent.ProgressPercent = 0.0f;
        Agent.CurrentTask = TEXT("Waiting for activation");
        Agent.Outputs.Empty();
    }
    
    // Set Studio Director (Agent #1) as active
    if (AgentWorkItems.IsValidIndex(0))
    {
        AgentWorkItems[0].Status = EAgentStatus::Working;
        AgentWorkItems[0].CurrentTask = InitialDirective;
        AgentWorkItems[0].ProgressPercent = 10.0f;
    }
    
    UpdateMetrics();
    LogProductionEvent(FString::Printf(TEXT("Production cycle %s started with directive: %s"), *CycleID, *InitialDirective));
}

void UProductionPipelineController::AdvanceToNextAgent()
{
    if (!bProductionActive || bEmergencyStop)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionPipelineController: Cannot advance - production not active"));
        return;
    }
    
    // Mark current agent as complete
    if (AgentWorkItems.IsValidIndex(CurrentAgentIndex))
    {
        AgentWorkItems[CurrentAgentIndex].Status = EAgentStatus::Complete;
        AgentWorkItems[CurrentAgentIndex].ProgressPercent = 100.0f;
        AgentWorkItems[CurrentAgentIndex].CurrentTask = TEXT("Work completed");
    }
    
    // Move to next agent
    CurrentAgentIndex++;
    
    if (CurrentAgentIndex >= AgentWorkItems.Num())
    {
        // Production cycle complete
        bProductionActive = false;
        LogProductionEvent(TEXT("Production cycle completed - all agents finished"));
        
        // Notify Studio Director
        if (StudioDirector)
        {
            // StudioDirector->OnProductionCycleComplete(CurrentCycleID);
        }
        return;
    }
    
    // Check dependencies for next agent
    if (!CheckDependencies(CurrentAgentIndex + 1)) // AgentID is 1-based
    {
        BlockAgent(CurrentAgentIndex + 1, TEXT("Dependencies not met"));
        return;
    }
    
    // Activate next agent
    if (AgentWorkItems.IsValidIndex(CurrentAgentIndex))
    {
        AgentWorkItems[CurrentAgentIndex].Status = EAgentStatus::Working;
        AgentWorkItems[CurrentAgentIndex].CurrentTask = TEXT("Processing work items");
        AgentWorkItems[CurrentAgentIndex].ProgressPercent = 5.0f;
    }
    
    UpdateMetrics();
    LogProductionEvent(FString::Printf(TEXT("Advanced to Agent #%d: %s"), 
        CurrentAgentIndex + 1, 
        AgentWorkItems.IsValidIndex(CurrentAgentIndex) ? *AgentWorkItems[CurrentAgentIndex].AgentName : TEXT("Unknown")));
}

void UProductionPipelineController::BlockAgent(int32 AgentID, const FString& Reason)
{
    int32 AgentIndex = AgentID - 1; // Convert to 0-based index
    if (!AgentWorkItems.IsValidIndex(AgentIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("ProductionPipelineController: Invalid AgentID %d"), AgentID);
        return;
    }
    
    AgentWorkItems[AgentIndex].Status = EAgentStatus::Blocked;
    AgentWorkItems[AgentIndex].CurrentTask = FString::Printf(TEXT("BLOCKED: %s"), *Reason);
    
    UpdateMetrics();
    LogProductionEvent(FString::Printf(TEXT("Agent #%d (%s) blocked: %s"), 
        AgentID, *AgentWorkItems[AgentIndex].AgentName, *Reason));
    
    // If this is a critical blocker, escalate to Studio Director
    if (Reason.Contains(TEXT("QA")) || Reason.Contains(TEXT("Architecture")))
    {
        if (StudioDirector)
        {
            // StudioDirector->OnCriticalBlock(AgentID, Reason);
        }
    }
}

void UProductionPipelineController::UnblockAgent(int32 AgentID)
{
    int32 AgentIndex = AgentID - 1;
    if (!AgentWorkItems.IsValidIndex(AgentIndex))
    {
        return;
    }
    
    if (AgentWorkItems[AgentIndex].Status == EAgentStatus::Blocked)
    {
        AgentWorkItems[AgentIndex].Status = EAgentStatus::Working;
        AgentWorkItems[AgentIndex].CurrentTask = TEXT("Resuming work");
        
        UpdateMetrics();
        LogProductionEvent(FString::Printf(TEXT("Agent #%d (%s) unblocked"), 
            AgentID, *AgentWorkItems[AgentIndex].AgentName));
    }
}

void UProductionPipelineController::CompleteAgentWork(int32 AgentID, const TArray<FString>& Outputs)
{
    int32 AgentIndex = AgentID - 1;
    if (!AgentWorkItems.IsValidIndex(AgentIndex))
    {
        return;
    }
    
    // Validate outputs
    if (!ValidateAgentOutput(AgentID, Outputs))
    {
        BlockAgent(AgentID, TEXT("Output validation failed"));
        return;
    }
    
    AgentWorkItems[AgentIndex].Outputs = Outputs;
    AgentWorkItems[AgentIndex].Status = EAgentStatus::Complete;
    AgentWorkItems[AgentIndex].ProgressPercent = 100.0f;
    AgentWorkItems[AgentIndex].CurrentTask = TEXT("Work completed successfully");
    
    UpdateMetrics();
    LogProductionEvent(FString::Printf(TEXT("Agent #%d (%s) completed work with %d outputs"), 
        AgentID, *AgentWorkItems[AgentIndex].AgentName, Outputs.Num()));
    
    // Auto-advance if this is the current agent
    if (AgentIndex == CurrentAgentIndex)
    {
        AdvanceToNextAgent();
    }
}

bool UProductionPipelineController::ValidatePhaseCompletion(EProductionPhase Phase)
{
    // Define which agents must be complete for each phase
    TArray<int32> RequiredAgents;
    
    switch (Phase)
    {
        case EProductionPhase::TechnicalFoundation:
            RequiredAgents = {1, 2, 3, 4}; // Studio Director, Engine Architect, Core Systems, Performance
            break;
        case EProductionPhase::WorldGeneration:
            RequiredAgents = {5, 6, 7, 8}; // PCG, Environment, Architecture, Lighting
            break;
        case EProductionPhase::CharacterSystems:
            RequiredAgents = {9, 10, 11, 12, 13}; // Characters, Animation, NPC, Combat, Crowd
            break;
        case EProductionPhase::GameplayCore:
            RequiredAgents = {14, 15}; // Quest, Narrative
            break;
        case EProductionPhase::AudioVisual:
            RequiredAgents = {16, 17}; // Audio, VFX
            break;
        case EProductionPhase::QualityAssurance:
            RequiredAgents = {18}; // QA
            break;
        case EProductionPhase::Integration:
            RequiredAgents = {19}; // Integration & Build
            break;
        default:
            return true;
    }
    
    // Check if all required agents are complete
    for (int32 AgentID : RequiredAgents)
    {
        int32 AgentIndex = AgentID - 1;
        if (AgentWorkItems.IsValidIndex(AgentIndex))
        {
            if (AgentWorkItems[AgentIndex].Status != EAgentStatus::Complete)
            {
                return false;
            }
        }
    }
    
    return true;
}

bool UProductionPipelineController::CheckDependencies(int32 AgentID)
{
    if (!AgentDependencies.Contains(AgentID))
    {
        return true; // No dependencies
    }
    
    const TArray<int32>& Dependencies = AgentDependencies[AgentID];
    for (int32 DepAgentID : Dependencies)
    {
        int32 DepAgentIndex = DepAgentID - 1;
        if (AgentWorkItems.IsValidIndex(DepAgentIndex))
        {
            if (AgentWorkItems[DepAgentIndex].Status != EAgentStatus::Complete)
            {
                return false;
            }
        }
    }
    
    return true;
}

FProductionMetrics UProductionPipelineController::GetProductionMetrics() const
{
    return CurrentMetrics;
}

TArray<FAgentWorkItem> UProductionPipelineController::GetAgentStatuses() const
{
    return AgentWorkItems;
}

FAgentWorkItem UProductionPipelineController::GetAgentStatus(int32 AgentID) const
{
    int32 AgentIndex = AgentID - 1;
    if (AgentWorkItems.IsValidIndex(AgentIndex))
    {
        return AgentWorkItems[AgentIndex];
    }
    
    return FAgentWorkItem(); // Return default
}

void UProductionPipelineController::EmergencyStop(const FString& Reason)
{
    UE_LOG(LogTemp, Error, TEXT("ProductionPipelineController: EMERGENCY STOP - %s"), *Reason);
    
    bEmergencyStop = true;
    bProductionActive = false;
    
    // Mark all working agents as blocked
    for (FAgentWorkItem& Agent : AgentWorkItems)
    {
        if (Agent.Status == EAgentStatus::Working)
        {
            Agent.Status = EAgentStatus::Blocked;
            Agent.CurrentTask = FString::Printf(TEXT("EMERGENCY STOP: %s"), *Reason);
        }
    }
    
    UpdateMetrics();
    LogProductionEvent(FString::Printf(TEXT("EMERGENCY STOP: %s"), *Reason));
}

void UProductionPipelineController::RollbackToLastStableState()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionPipelineController: Rolling back to last stable state"));
    
    // Find last completed agent
    int32 LastCompleteIndex = -1;
    for (int32 i = AgentWorkItems.Num() - 1; i >= 0; i--)
    {
        if (AgentWorkItems[i].Status == EAgentStatus::Complete)
        {
            LastCompleteIndex = i;
            break;
        }
    }
    
    if (LastCompleteIndex >= 0)
    {
        CurrentAgentIndex = LastCompleteIndex;
        
        // Reset all agents after the last complete one
        for (int32 i = LastCompleteIndex + 1; i < AgentWorkItems.Num(); i++)
        {
            AgentWorkItems[i].Status = EAgentStatus::Idle;
            AgentWorkItems[i].ProgressPercent = 0.0f;
            AgentWorkItems[i].CurrentTask = TEXT("Reset to idle");
            AgentWorkItems[i].Outputs.Empty();
        }
        
        bEmergencyStop = false;
        UpdateMetrics();
        LogProductionEvent(FString::Printf(TEXT("Rolled back to Agent #%d"), LastCompleteIndex + 1));
    }
}

void UProductionPipelineController::ForceAgentSequence(const TArray<int32>& AgentOrder)
{
    if (AgentOrder.Num() != 19)
    {
        UE_LOG(LogTemp, Error, TEXT("ProductionPipelineController: Invalid agent sequence - must contain all 19 agents"));
        return;
    }
    
    ProductionSequence = AgentOrder;
    LogProductionEvent(TEXT("Agent execution sequence overridden"));
}

void UProductionPipelineController::InitializeAgentDefinitions()
{
    AgentWorkItems.Empty();
    AgentDependencies.Empty();
    ProductionSequence.Empty();
    
    // Initialize all 19 agents
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
        FAgentWorkItem Agent;
        Agent.AgentID = i + 1;
        Agent.AgentName = AgentNames[i];
        Agent.Status = EAgentStatus::Idle;
        Agent.CurrentTask = TEXT("Awaiting activation");
        Agent.ProgressPercent = 0.0f;
        
        AgentWorkItems.Add(Agent);
        ProductionSequence.Add(i + 1);
    }
    
    // Set up dependencies
    AgentDependencies.Add(2, {1}); // Engine Architect depends on Studio Director
    AgentDependencies.Add(3, {2}); // Core Systems depends on Engine Architect
    AgentDependencies.Add(4, {3}); // Performance depends on Core Systems
    AgentDependencies.Add(5, {2}); // World Generator depends on Engine Architect
    AgentDependencies.Add(6, {5}); // Environment depends on World Generator
    AgentDependencies.Add(7, {6}); // Architecture depends on Environment
    AgentDependencies.Add(8, {7}); // Lighting depends on Architecture
    AgentDependencies.Add(9, {2}); // Character Artist depends on Engine Architect
    AgentDependencies.Add(10, {9}); // Animation depends on Character Artist
    AgentDependencies.Add(11, {10}); // NPC Behavior depends on Animation
    AgentDependencies.Add(12, {11}); // Combat AI depends on NPC Behavior
    AgentDependencies.Add(13, {12}); // Crowd Simulation depends on Combat AI
    AgentDependencies.Add(14, {15}); // Quest Designer depends on Narrative
    AgentDependencies.Add(15, {1}); // Narrative depends on Studio Director
    AgentDependencies.Add(16, {8}); // Audio depends on Lighting (world complete)
    AgentDependencies.Add(17, {16}); // VFX depends on Audio
    AgentDependencies.Add(18, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}); // QA depends on everyone
    AgentDependencies.Add(19, {18}); // Integration depends on QA
    
    UpdateMetrics();
}

void UProductionPipelineController::UpdateMetrics()
{
    CurrentMetrics.TotalAgents = AgentWorkItems.Num();
    CurrentMetrics.ActiveAgents = 0;
    CurrentMetrics.CompletedAgents = 0;
    CurrentMetrics.BlockedAgents = 0;
    
    for (const FAgentWorkItem& Agent : AgentWorkItems)
    {
        switch (Agent.Status)
        {
            case EAgentStatus::Working:
                CurrentMetrics.ActiveAgents++;
                break;
            case EAgentStatus::Complete:
                CurrentMetrics.CompletedAgents++;
                break;
            case EAgentStatus::Blocked:
                CurrentMetrics.BlockedAgents++;
                break;
            default:
                break;
        }
    }
    
    CurrentMetrics.OverallProgress = CurrentMetrics.TotalAgents > 0 ? 
        (float)CurrentMetrics.CompletedAgents / (float)CurrentMetrics.TotalAgents * 100.0f : 0.0f;
    
    CurrentMetrics.LastUpdate = FDateTime::Now().ToString();
    
    // Determine current phase
    if (CurrentMetrics.CompletedAgents <= 4)
        CurrentMetrics.CurrentPhase = EProductionPhase::TechnicalFoundation;
    else if (CurrentMetrics.CompletedAgents <= 8)
        CurrentMetrics.CurrentPhase = EProductionPhase::WorldGeneration;
    else if (CurrentMetrics.CompletedAgents <= 13)
        CurrentMetrics.CurrentPhase = EProductionPhase::CharacterSystems;
    else if (CurrentMetrics.CompletedAgents <= 15)
        CurrentMetrics.CurrentPhase = EProductionPhase::GameplayCore;
    else if (CurrentMetrics.CompletedAgents <= 17)
        CurrentMetrics.CurrentPhase = EProductionPhase::AudioVisual;
    else if (CurrentMetrics.CompletedAgents <= 18)
        CurrentMetrics.CurrentPhase = EProductionPhase::QualityAssurance;
    else
        CurrentMetrics.CurrentPhase = EProductionPhase::Integration;
}

void UProductionPipelineController::LogProductionEvent(const FString& Event)
{
    FString LogMessage = FString::Printf(TEXT("[%s] %s"), *CurrentCycleID, *Event);
    UE_LOG(LogTemp, Warning, TEXT("ProductionPipeline: %s"), *LogMessage);
}

bool UProductionPipelineController::ValidateAgentOutput(int32 AgentID, const TArray<FString>& Outputs)
{
    // Basic validation - each agent should produce at least one output
    if (Outputs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionPipelineController: Agent #%d produced no outputs"), AgentID);
        return false;
    }
    
    // Agent-specific validation
    switch (AgentID)
    {
        case 2: // Engine Architect
            // Should produce architecture documents
            return Outputs.ContainsByPredicate([](const FString& Output) {
                return Output.Contains(TEXT("Architecture")) || Output.Contains(TEXT(".h")) || Output.Contains(TEXT(".cpp"));
            });
            
        case 18: // QA Agent
            // QA can block everything - special validation
            for (const FString& Output : Outputs)
            {
                if (Output.Contains(TEXT("BLOCK")) || Output.Contains(TEXT("FAIL")))
                {
                    return false; // QA found critical issues
                }
            }
            break;
            
        default:
            break;
    }
    
    return true;
}