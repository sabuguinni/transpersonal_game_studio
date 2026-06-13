#include "ProductionDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"

AProductionDirector::AProductionDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    CurrentPhase = EDir_ProductionPhase::Prototyping;
    bAutoUpdateMetrics = true;
    MetricsUpdateInterval = 5.0f;
    LastMetricsUpdate = 0.0f;
    bPipelineInitialized = false;
    
    // Initialize production metrics
    ProductionMetrics = FDir_ProductionMetrics();
}

void AProductionDirector::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bPipelineInitialized)
    {
        InitializeProductionPipeline();
    }
    
    // Start metrics calculation
    CalculateProductionMetrics();
}

void AProductionDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoUpdateMetrics)
    {
        LastMetricsUpdate += DeltaTime;
        if (LastMetricsUpdate >= MetricsUpdateInterval)
        {
            CalculateProductionMetrics();
            LastMetricsUpdate = 0.0f;
        }
    }
}

void AProductionDirector::InitializeProductionPipeline()
{
    if (bPipelineInitialized)
    {
        return;
    }
    
    CreateDefaultAgentPipeline();
    bPipelineInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionDirector: Pipeline initialized with %d agents"), AgentPipeline.Num());
}

void AProductionDirector::CreateDefaultAgentPipeline()
{
    AgentPipeline.Empty();
    
    // Define the 19-agent pipeline with dependencies
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
    
    // Create agent info structures
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentInfo AgentInfo;
        AgentInfo.AgentID = i + 1;
        AgentInfo.AgentName = AgentNames[i];
        AgentInfo.Status = EDir_AgentStatus::Working;
        AgentInfo.ProgressPercentage = (i < 5) ? 75.0f : 25.0f; // First 5 agents more advanced
        AgentInfo.CurrentTask = TEXT("Implementing core systems");
        
        // Set dependencies based on agent hierarchy
        if (i > 0)
        {
            AgentInfo.Dependencies.Add(FString::Printf(TEXT("Agent_%d"), i));
        }
        
        AgentPipeline.Add(AgentInfo);
    }
}

void AProductionDirector::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, float Progress)
{
    for (FDir_AgentInfo& Agent : AgentPipeline)
    {
        if (Agent.AgentID == AgentID)
        {
            EDir_AgentStatus OldStatus = Agent.Status;
            Agent.Status = NewStatus;
            Agent.ProgressPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            Agent.LastUpdate = FDateTime::Now();
            
            // Trigger Blueprint event
            OnAgentStatusChanged(AgentID, NewStatus);
            
            UE_LOG(LogTemp, Log, TEXT("Agent %d (%s) status changed: %d -> %d, Progress: %.1f%%"), 
                   AgentID, *Agent.AgentName, (int32)OldStatus, (int32)NewStatus, Progress);
            
            break;
        }
    }
    
    // Recalculate metrics after status change
    CalculateProductionMetrics();
}

void AProductionDirector::SetAgentTask(int32 AgentID, const FString& TaskDescription)
{
    for (FDir_AgentInfo& Agent : AgentPipeline)
    {
        if (Agent.AgentID == AgentID)
        {
            Agent.CurrentTask = TaskDescription;
            Agent.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Log, TEXT("Agent %d (%s) assigned task: %s"), 
                   AgentID, *Agent.AgentName, *TaskDescription);
            break;
        }
    }
}

void AProductionDirector::AdvanceProductionPhase()
{
    EDir_ProductionPhase OldPhase = CurrentPhase;
    
    // Check if current phase requirements are met
    float OverallProgress = CalculateOverallProgress();
    
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            if (OverallProgress >= 20.0f)
            {
                CurrentPhase = EDir_ProductionPhase::Prototyping;
            }
            break;
            
        case EDir_ProductionPhase::Prototyping:
            if (OverallProgress >= 40.0f)
            {
                CurrentPhase = EDir_ProductionPhase::Production;
            }
            break;
            
        case EDir_ProductionPhase::Production:
            if (OverallProgress >= 80.0f)
            {
                CurrentPhase = EDir_ProductionPhase::Testing;
            }
            break;
            
        case EDir_ProductionPhase::Testing:
            if (OverallProgress >= 95.0f)
            {
                CurrentPhase = EDir_ProductionPhase::Polish;
            }
            break;
            
        case EDir_ProductionPhase::Polish:
            if (OverallProgress >= 100.0f)
            {
                CurrentPhase = EDir_ProductionPhase::Release;
            }
            break;
            
        default:
            break;
    }
    
    if (CurrentPhase != OldPhase)
    {
        OnPhaseChanged(CurrentPhase);
        UE_LOG(LogTemp, Warning, TEXT("Production phase advanced: %d -> %d"), (int32)OldPhase, (int32)CurrentPhase);
    }
}

void AProductionDirector::CalculateProductionMetrics()
{
    ProductionMetrics.OverallProgress = CalculateOverallProgress();
    ProductionMetrics.TotalSystems = AgentPipeline.Num();
    ProductionMetrics.CompletedSystems = CountAgentsByStatus(EDir_AgentStatus::Complete);
    ProductionMetrics.ActiveAgents = CountAgentsByStatus(EDir_AgentStatus::Working);
    ProductionMetrics.BlockedAgents = CountAgentsByStatus(EDir_AgentStatus::Blocked);
    ProductionMetrics.LastMetricsUpdate = FDateTime::Now();
    
    // Calculate estimated time to completion (simplified)
    if (ProductionMetrics.OverallProgress > 0.0f)
    {
        float RemainingWork = 100.0f - ProductionMetrics.OverallProgress;
        ProductionMetrics.EstimatedTimeToCompletion = (RemainingWork / ProductionMetrics.OverallProgress) * 24.0f; // Hours
    }
    
    // Check for milestone events
    if (ProductionMetrics.OverallProgress >= 25.0f && ProductionMetrics.OverallProgress < 30.0f)
    {
        OnProductionMilestone(25.0f);
    }
    else if (ProductionMetrics.OverallProgress >= 50.0f && ProductionMetrics.OverallProgress < 55.0f)
    {
        OnProductionMilestone(50.0f);
    }
    else if (ProductionMetrics.OverallProgress >= 75.0f && ProductionMetrics.OverallProgress < 80.0f)
    {
        OnProductionMilestone(75.0f);
    }
}

float AProductionDirector::CalculateOverallProgress()
{
    if (AgentPipeline.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_AgentInfo& Agent : AgentPipeline)
    {
        TotalProgress += Agent.ProgressPercentage;
    }
    
    return TotalProgress / AgentPipeline.Num();
}

int32 AProductionDirector::CountAgentsByStatus(EDir_AgentStatus Status)
{
    int32 Count = 0;
    for (const FDir_AgentInfo& Agent : AgentPipeline)
    {
        if (Agent.Status == Status)
        {
            Count++;
        }
    }
    return Count;
}

bool AProductionDirector::ValidateAgentDependencies(int32 AgentID)
{
    for (const FDir_AgentInfo& Agent : AgentPipeline)
    {
        if (Agent.AgentID == AgentID)
        {
            // Check if all dependencies are complete
            for (const FString& Dependency : Agent.Dependencies)
            {
                // Parse dependency ID and check status
                FString DependencyIDStr = Dependency.Replace(TEXT("Agent_"), TEXT(""));
                int32 DependencyID = FCString::Atoi(*DependencyIDStr);
                
                for (const FDir_AgentInfo& DepAgent : AgentPipeline)
                {
                    if (DepAgent.AgentID == DependencyID)
                    {
                        if (DepAgent.Status != EDir_AgentStatus::Complete && DepAgent.ProgressPercentage < 80.0f)
                        {
                            return false;
                        }
                        break;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

TArray<int32> AProductionDirector::GetBlockedAgents()
{
    TArray<int32> BlockedAgents;
    
    for (const FDir_AgentInfo& Agent : AgentPipeline)
    {
        if (Agent.Status == EDir_AgentStatus::Blocked || !ValidateAgentDependencies(Agent.AgentID))
        {
            BlockedAgents.Add(Agent.AgentID);
        }
    }
    
    return BlockedAgents;
}

FString AProductionDirector::GetProductionSummary()
{
    FString Summary;
    Summary += FString::Printf(TEXT("=== PRODUCTION SUMMARY ===\n"));
    Summary += FString::Printf(TEXT("Current Phase: %d\n"), (int32)CurrentPhase);
    Summary += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), ProductionMetrics.OverallProgress);
    Summary += FString::Printf(TEXT("Completed Systems: %d/%d\n"), ProductionMetrics.CompletedSystems, ProductionMetrics.TotalSystems);
    Summary += FString::Printf(TEXT("Active Agents: %d\n"), ProductionMetrics.ActiveAgents);
    Summary += FString::Printf(TEXT("Blocked Agents: %d\n"), ProductionMetrics.BlockedAgents);
    Summary += FString::Printf(TEXT("Est. Time to Completion: %.1f hours\n"), ProductionMetrics.EstimatedTimeToCompletion);
    
    return Summary;
}

void AProductionDirector::EditorRefreshMetrics()
{
    CalculateProductionMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Production metrics refreshed from editor"));
}

void AProductionDirector::EditorResetPipeline()
{
    bPipelineInitialized = false;
    AgentPipeline.Empty();
    InitializeProductionPipeline();
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline reset from editor"));
}

void AProductionDirector::EditorGenerateReport()
{
    FString Report = GetProductionSummary();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    // Also print to screen for editor visibility
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, Report);
    }
}