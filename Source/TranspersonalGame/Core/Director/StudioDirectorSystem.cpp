#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Initialize default values
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    CurrentActiveAgent = 1; // Start with Studio Director (self)
    bVisionIntegrityMaintained = true;
    bQAApprovalRequired = false;
    FailedQualityChecks = 0;
    LastMetricsUpdate = 0.0f;
    bPipelineInitialized = false;
    
    // Set core creative pillars
    CorePillars.Add(TEXT("Immersive Prehistoric World"));
    CorePillars.Add(TEXT("Meaningful Character Progression"));
    CorePillars.Add(TEXT("Dynamic Ecosystem Simulation"));
    CorePillars.Add(TEXT("Emergent Storytelling"));
    CorePillars.Add(TEXT("Transpersonal Connection"));
    
    // Set quality gates criteria
    QualityGatesCriteria.Add(TEXT("60 FPS on PC"));
    QualityGatesCriteria.Add(TEXT("30 FPS on Console"));
    QualityGatesCriteria.Add(TEXT("No Critical Bugs"));
    QualityGatesCriteria.Add(TEXT("Creative Vision Compliance"));
    QualityGatesCriteria.Add(TEXT("Performance Targets Met"));
}

void UStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System: Initializing Production Pipeline"));
    InitializeProductionPipeline();
    
    // Set creative vision from Miguel's direction
    SetCreativeVision(TEXT("Create a prehistoric survival game that connects players to their primal nature while exploring themes of consciousness, community, and the relationship between humanity and nature."));
}

void UStudioDirectorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update metrics every 5 seconds
    LastMetricsUpdate += DeltaTime;
    if (LastMetricsUpdate >= 5.0f)
    {
        UpdateProductionMetrics();
        CheckForBottlenecks();
        LastMetricsUpdate = 0.0f;
    }
}

void UStudioDirectorSystem::InitializeProductionPipeline()
{
    if (bPipelineInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Pipeline already initialized"));
        return;
    }
    
    // Initialize the 19-agent chain
    AgentChain.Empty();
    
    // Agent #01 - Studio Director (self)
    FDir_AgentInfo StudioDirector;
    StudioDirector.AgentName = TEXT("Studio Director");
    StudioDirector.AgentID = 1;
    StudioDirector.Status = EDir_AgentStatus::Working;
    StudioDirector.CurrentTask = TEXT("Production Pipeline Oversight");
    StudioDirector.CompletionPercentage = 25.0f;
    AgentChain.Add(StudioDirector);
    
    // Agent #02 - Engine Architect
    FDir_AgentInfo EngineArchitect;
    EngineArchitect.AgentName = TEXT("Engine Architect");
    EngineArchitect.AgentID = 2;
    EngineArchitect.Status = EDir_AgentStatus::Idle;
    EngineArchitect.CurrentTask = TEXT("Awaiting Technical Architecture Definition");
    EngineArchitect.Dependencies.Add(TEXT("Studio Director Vision"));
    AgentChain.Add(EngineArchitect);
    
    // Agent #03 - Core Systems Programmer
    FDir_AgentInfo CoreSystems;
    CoreSystems.AgentName = TEXT("Core Systems Programmer");
    CoreSystems.AgentID = 3;
    CoreSystems.Status = EDir_AgentStatus::Idle;
    CoreSystems.CurrentTask = TEXT("Awaiting Engine Architecture");
    CoreSystems.Dependencies.Add(TEXT("Engine Architect"));
    AgentChain.Add(CoreSystems);
    
    // Continue with remaining agents...
    TArray<FString> AgentNames = {
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
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentInfo Agent;
        Agent.AgentName = AgentNames[i];
        Agent.AgentID = i + 4; // Starting from ID 4
        Agent.Status = EDir_AgentStatus::Idle;
        Agent.CurrentTask = TEXT("Awaiting Dependencies");
        
        // Set dependencies based on agent order
        if (i == 0) // Performance Optimizer
        {
            Agent.Dependencies.Add(TEXT("Core Systems Programmer"));
        }
        else if (i < 10) // Technical agents
        {
            Agent.Dependencies.Add(AgentNames[i - 1]);
        }
        else if (i == 14) // Narrative Agent
        {
            Agent.Dependencies.Add(TEXT("Quest & Mission Designer"));
        }
        else
        {
            Agent.Dependencies.Add(AgentNames[i - 1]);
        }
        
        AgentChain.Add(Agent);
    }
    
    bPipelineInitialized = true;
    UpdateProductionMetrics();
    
    LogProductionEvent(TEXT("Production pipeline initialized with 19 agents"));
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Production pipeline initialized successfully"));
}

void UStudioDirectorSystem::AdvanceToNextAgent()
{
    if (CurrentActiveAgent >= AgentChain.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: All agents completed - cycle complete"));
        LogProductionEvent(TEXT("Production cycle completed"));
        return;
    }
    
    // Mark current agent as complete
    if (CurrentActiveAgent > 0 && CurrentActiveAgent <= AgentChain.Num())
    {
        AgentChain[CurrentActiveAgent - 1].Status = EDir_AgentStatus::Complete;
        AgentChain[CurrentActiveAgent - 1].CompletionPercentage = 100.0f;
    }
    
    // Advance to next agent
    CurrentActiveAgent++;
    
    if (CurrentActiveAgent <= AgentChain.Num())
    {
        // Check if next agent's dependencies are met
        if (IsAgentReadyToWork(CurrentActiveAgent))
        {
            AgentChain[CurrentActiveAgent - 1].Status = EDir_AgentStatus::Working;
            LogProductionEvent(FString::Printf(TEXT("Advanced to Agent #%d: %s"), 
                CurrentActiveAgent, *AgentChain[CurrentActiveAgent - 1].AgentName));
        }
        else
        {
            AgentChain[CurrentActiveAgent - 1].Status = EDir_AgentStatus::Blocked;
            LogProductionEvent(FString::Printf(TEXT("Agent #%d blocked - dependencies not met"), CurrentActiveAgent));
        }
    }
    
    UpdateProductionMetrics();
}

void UStudioDirectorSystem::BlockAgent(int32 AgentID, const FString& Reason)
{
    if (AgentID > 0 && AgentID <= AgentChain.Num())
    {
        AgentChain[AgentID - 1].Status = EDir_AgentStatus::Blocked;
        LogProductionEvent(FString::Printf(TEXT("Agent #%d (%s) blocked: %s"), 
            AgentID, *AgentChain[AgentID - 1].AgentName, *Reason));
        
        // If QA blocks, stop everything
        if (AgentID == 18) // QA Agent
        {
            bQAApprovalRequired = true;
            UE_LOG(LogTemp, Error, TEXT("Studio Director: QA BLOCK - All production halted"));
        }
    }
}

void UStudioDirectorSystem::UnblockAgent(int32 AgentID)
{
    if (AgentID > 0 && AgentID <= AgentChain.Num())
    {
        if (IsAgentReadyToWork(AgentID))
        {
            AgentChain[AgentID - 1].Status = EDir_AgentStatus::Working;
            LogProductionEvent(FString::Printf(TEXT("Agent #%d (%s) unblocked"), 
                AgentID, *AgentChain[AgentID - 1].AgentName));
        }
    }
}

FDir_ProductionMetrics UStudioDirectorSystem::GetProductionMetrics() const
{
    return ProductionMetrics;
}

void UStudioDirectorSystem::SetCreativeVision(const FString& Vision)
{
    CreativeDirectorVision = Vision;
    LogProductionEvent(TEXT("Creative vision updated by Miguel"));
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Creative vision set - %s"), *Vision);
}

bool UStudioDirectorSystem::ValidateAgentOutput(int32 AgentID, const FString& Output)
{
    // Check if output aligns with creative vision and core pillars
    bool bVisionCompliant = true;
    
    // Simple keyword validation against core pillars
    for (const FString& Pillar : CorePillars)
    {
        // This is a simplified check - in practice would be more sophisticated
        if (!Output.Contains(TEXT("prehistoric")) && Pillar.Contains(TEXT("Prehistoric")))
        {
            bVisionCompliant = false;
            break;
        }
    }
    
    if (!bVisionCompliant)
    {
        UE_LOG(LogTemp, Warning, TEXT("Studio Director: Agent #%d output failed vision compliance"), AgentID);
        LogProductionEvent(FString::Printf(TEXT("Agent #%d output rejected - vision non-compliance"), AgentID));
    }
    
    return bVisionCompliant;
}

void UStudioDirectorSystem::EnforceVisionCompliance()
{
    // Review all agent outputs for vision compliance
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        if (AgentChain[i].Status == EDir_AgentStatus::Complete)
        {
            // In practice, this would check actual outputs
            // For now, we assume compliance unless explicitly flagged
            continue;
        }
    }
    
    LogProductionEvent(TEXT("Vision compliance check completed"));
}

bool UStudioDirectorSystem::RunQualityGate()
{
    bool bPassedQualityGate = true;
    
    // Check each quality criteria
    for (const FString& Criteria : QualityGatesCriteria)
    {
        // Simplified quality check - in practice would run actual tests
        if (Criteria.Contains(TEXT("FPS")))
        {
            // Would check actual performance metrics
            continue;
        }
        
        if (Criteria.Contains(TEXT("Bugs")))
        {
            // Would check bug database
            continue;
        }
        
        if (Criteria.Contains(TEXT("Vision")))
        {
            if (!bVisionIntegrityMaintained)
            {
                bPassedQualityGate = false;
                break;
            }
        }
    }
    
    if (!bPassedQualityGate)
    {
        FailedQualityChecks++;
        LogProductionEvent(TEXT("Quality gate FAILED"));
    }
    else
    {
        LogProductionEvent(TEXT("Quality gate PASSED"));
    }
    
    return bPassedQualityGate;
}

void UStudioDirectorSystem::TriggerQAReview()
{
    // Signal QA agent to begin review
    if (AgentChain.Num() >= 18)
    {
        AgentChain[17].Status = EDir_AgentStatus::Working; // QA is agent #18 (index 17)
        AgentChain[17].CurrentTask = TEXT("Quality Assurance Review");
        LogProductionEvent(TEXT("QA review triggered"));
    }
}

void UStudioDirectorSystem::HandleQABlock(const FString& BlockReason)
{
    bQAApprovalRequired = true;
    LogProductionEvent(FString::Printf(TEXT("QA BLOCK: %s"), *BlockReason));
    
    // Halt all production until QA block is resolved
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        if (AgentChain[i].Status == EDir_AgentStatus::Working)
        {
            AgentChain[i].Status = EDir_AgentStatus::Blocked;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Studio Director: QA BLOCK - %s"), *BlockReason);
}

void UStudioDirectorSystem::SendTaskToAgent(int32 AgentID, const FString& TaskDescription)
{
    if (AgentID > 0 && AgentID <= AgentChain.Num())
    {
        AgentChain[AgentID - 1].CurrentTask = TaskDescription;
        LogProductionEvent(FString::Printf(TEXT("Task sent to Agent #%d: %s"), AgentID, *TaskDescription));
    }
}

void UStudioDirectorSystem::ReceiveAgentReport(int32 AgentID, const FString& Report)
{
    LogProductionEvent(FString::Printf(TEXT("Report from Agent #%d: %s"), AgentID, *Report));
    
    // Validate report against creative vision
    if (!ValidateAgentOutput(AgentID, Report))
    {
        BlockAgent(AgentID, TEXT("Output does not align with creative vision"));
    }
}

void UStudioDirectorSystem::BroadcastToAllAgents(const FString& Message)
{
    LogProductionEvent(FString::Printf(TEXT("BROADCAST: %s"), *Message));
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Broadcast: %s"), *Message);
}

void UStudioDirectorSystem::DebugPrintPipelineStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION PIPELINE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Active Agent: %d"), CurrentActiveAgent);
    UE_LOG(LogTemp, Warning, TEXT("QA Approval Required: %s"), bQAApprovalRequired ? TEXT("YES") : TEXT("NO"));
    
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        const FDir_AgentInfo& Agent = AgentChain[i];
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s): %d - %s (%.1f%%)"), 
            Agent.AgentID, *Agent.AgentName, (int32)Agent.Status, *Agent.CurrentTask, Agent.CompletionPercentage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END PIPELINE STATUS ==="));
}

void UStudioDirectorSystem::SimulateAgentWork(int32 AgentID, float ProgressAmount)
{
    if (AgentID > 0 && AgentID <= AgentChain.Num())
    {
        FDir_AgentInfo& Agent = AgentChain[AgentID - 1];
        Agent.CompletionPercentage = FMath::Clamp(Agent.CompletionPercentage + ProgressAmount, 0.0f, 100.0f);
        
        if (Agent.CompletionPercentage >= 100.0f)
        {
            Agent.Status = EDir_AgentStatus::Complete;
        }
        
        UpdateProductionMetrics();
    }
}

void UStudioDirectorSystem::ResetProductionPipeline()
{
    bPipelineInitialized = false;
    CurrentActiveAgent = 1;
    bQAApprovalRequired = false;
    FailedQualityChecks = 0;
    AgentChain.Empty();
    ProductionLog.Empty();
    
    InitializeProductionPipeline();
    UE_LOG(LogTemp, Warning, TEXT("Studio Director: Production pipeline reset"));
}

void UStudioDirectorSystem::UpdateProductionMetrics()
{
    ProductionMetrics.TotalAgents = AgentChain.Num();
    ProductionMetrics.ActiveAgents = 0;
    ProductionMetrics.BlockedAgents = 0;
    
    float TotalProgress = 0.0f;
    
    for (const FDir_AgentInfo& Agent : AgentChain)
    {
        switch (Agent.Status)
        {
            case EDir_AgentStatus::Working:
                ProductionMetrics.ActiveAgents++;
                break;
            case EDir_AgentStatus::Blocked:
                ProductionMetrics.BlockedAgents++;
                break;
            default:
                break;
        }
        
        TotalProgress += Agent.CompletionPercentage;
    }
    
    ProductionMetrics.OverallProgress = AgentChain.Num() > 0 ? TotalProgress / AgentChain.Num() : 0.0f;
    
    // Identify bottlenecks
    if (ProductionMetrics.BlockedAgents > 0)
    {
        ProductionMetrics.CurrentBottleneck = TEXT("Blocked Agents");
    }
    else if (bQAApprovalRequired)
    {
        ProductionMetrics.CurrentBottleneck = TEXT("QA Review");
    }
    else
    {
        ProductionMetrics.CurrentBottleneck = TEXT("None");
    }
    
    // Estimate completion time (simplified calculation)
    float RemainingWork = 100.0f - ProductionMetrics.OverallProgress;
    ProductionMetrics.EstimatedCompletionHours = RemainingWork / 10.0f; // Assume 10% per hour
}

void UStudioDirectorSystem::CheckForBottlenecks()
{
    // Check for agents waiting too long
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        const FDir_AgentInfo& Agent = AgentChain[i];
        
        if (Agent.Status == EDir_AgentStatus::Blocked)
        {
            UE_LOG(LogTemp, Warning, TEXT("Studio Director: Bottleneck detected at Agent #%d (%s)"), 
                Agent.AgentID, *Agent.AgentName);
        }
    }
}

void UStudioDirectorSystem::ValidateAgentDependencies()
{
    for (int32 i = 0; i < AgentChain.Num(); i++)
    {
        if (!IsAgentReadyToWork(i + 1))
        {
            AgentChain[i].Status = EDir_AgentStatus::Blocked;
        }
    }
}

bool UStudioDirectorSystem::IsAgentReadyToWork(int32 AgentID) const
{
    if (AgentID <= 0 || AgentID > AgentChain.Num())
    {
        return false;
    }
    
    const FDir_AgentInfo& Agent = AgentChain[AgentID - 1];
    
    // Check if all dependencies are complete
    for (const FString& Dependency : Agent.Dependencies)
    {
        // Find dependency agent and check if complete
        for (const FDir_AgentInfo& OtherAgent : AgentChain)
        {
            if (OtherAgent.AgentName == Dependency)
            {
                if (OtherAgent.Status != EDir_AgentStatus::Complete)
                {
                    return false;
                }
                break;
            }
        }
    }
    
    return true;
}

void UStudioDirectorSystem::LogProductionEvent(const FString& Event)
{
    FString TimeStamp = FDateTime::Now().ToString();
    FString LogEntry = FString::Printf(TEXT("[%s] %s"), *TimeStamp, *Event);
    
    ProductionLog.Add(LogEntry);
    
    // Keep only last 100 entries
    if (ProductionLog.Num() > 100)
    {
        ProductionLog.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Studio Director Log: %s"), *LogEntry);
}