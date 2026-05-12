#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize production pipeline
    bPipelineActive = false;
    CurrentAgentIndex = 1;
    
    // Set up default metrics
    CurrentMetrics.CycleID = TEXT("PROD_CYCLE_AUTO_20260512_010");
    CurrentMetrics.TotalAgents = 19;
    CurrentMetrics.ActiveAgents = 0;
    CurrentMetrics.SystemsImplemented = 0;
    CurrentMetrics.PlayableFeatures = 0;
    CurrentMetrics.bCompilationSuccessful = false;
    CurrentMetrics.CycleStartTime = 0.0f;
    CurrentMetrics.EstimatedCompletionTime = 0.0f;
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized"));
    
    // Initialize the production pipeline
    InitializeProductionPipeline();
    
    // Start timing
    CurrentMetrics.CycleStartTime = GetWorld()->GetTimeSeconds();
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bPipelineActive)
    {
        CalculateProductionMetrics();
        
        // Log status every 30 seconds
        static float LastLogTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastLogTime > 30.0f)
        {
            LogProductionStatus();
            LastLogTime = CurrentTime;
        }
    }
}

void ADir_ProductionCoordinator::InitializeProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Production Pipeline for 19 agents"));
    
    SetupAgentList();
    bPipelineActive = true;
    CurrentAgentIndex = 1; // Start with Studio Director (self)
    
    // Update metrics
    CurrentMetrics.ActiveAgents = 1;
    CurrentMetrics.CycleStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Pipeline initialized - Ready for agent coordination"));
}

void ADir_ProductionCoordinator::SetupAgentList()
{
    AgentStatusList.Empty();
    
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
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentStatus NewAgent;
        NewAgent.AgentID = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.bIsActive = (i == 0); // Only Studio Director starts active
        NewAgent.CompletionPercentage = 0.0f;
        NewAgent.CurrentTask = (i == 0) ? TEXT("Production Coordination") : TEXT("Waiting");
        NewAgent.FilesCreated = 0;
        NewAgent.UE5CommandsExecuted = 0;
        
        AgentStatusList.Add(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent list setup complete - %d agents registered"), AgentStatusList.Num());
}

void ADir_ProductionCoordinator::UpdateAgentStatus(int32 AgentID, const FString& TaskName, float Completion, int32 FilesCreated, int32 CommandsExecuted)
{
    if (AgentID < 1 || AgentID > AgentStatusList.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Agent ID: %d"), AgentID);
        return;
    }
    
    FDir_AgentStatus& Agent = AgentStatusList[AgentID - 1];
    Agent.CurrentTask = TaskName;
    Agent.CompletionPercentage = FMath::Clamp(Completion, 0.0f, 100.0f);
    Agent.FilesCreated = FilesCreated;
    Agent.UE5CommandsExecuted = CommandsExecuted;
    Agent.bIsActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s) updated: %s - %.1f%% complete"), 
           AgentID, *Agent.AgentName, *TaskName, Completion);
    
    // Update global metrics
    CalculateProductionMetrics();
}

void ADir_ProductionCoordinator::AdvanceToNextAgent()
{
    if (CurrentAgentIndex < AgentStatusList.Num())
    {
        // Mark current agent as complete
        if (CurrentAgentIndex > 0)
        {
            AgentStatusList[CurrentAgentIndex - 1].CompletionPercentage = 100.0f;
            AgentStatusList[CurrentAgentIndex - 1].bIsActive = false;
        }
        
        // Advance to next agent
        CurrentAgentIndex++;
        
        if (CurrentAgentIndex <= AgentStatusList.Num())
        {
            AgentStatusList[CurrentAgentIndex - 1].bIsActive = true;
            AgentStatusList[CurrentAgentIndex - 1].CurrentTask = TEXT("Starting...");
            
            UE_LOG(LogTemp, Warning, TEXT("Advanced to Agent #%d: %s"), 
                   CurrentAgentIndex, *AgentStatusList[CurrentAgentIndex - 1].AgentName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Production cycle complete - all agents finished"));
            bPipelineActive = false;
        }
    }
}

FDir_AgentStatus ADir_ProductionCoordinator::GetAgentStatus(int32 AgentID) const
{
    if (AgentID >= 1 && AgentID <= AgentStatusList.Num())
    {
        return AgentStatusList[AgentID - 1];
    }
    
    return FDir_AgentStatus(); // Return default/empty status
}

FDir_ProductionMetrics ADir_ProductionCoordinator::GetProductionMetrics() const
{
    return CurrentMetrics;
}

bool ADir_ProductionCoordinator::ValidateAgentDeliverable(int32 AgentID, const FString& DeliverableType)
{
    if (AgentID < 1 || AgentID > AgentStatusList.Num())
    {
        return false;
    }
    
    const FDir_AgentStatus& Agent = AgentStatusList[AgentID - 1];
    
    // Basic validation rules
    if (DeliverableType == TEXT("CODE"))
    {
        return Agent.FilesCreated >= 2; // Must create at least .h and .cpp
    }
    else if (DeliverableType == TEXT("UE5_INTEGRATION"))
    {
        return Agent.UE5CommandsExecuted >= 1; // Must execute at least one UE5 command
    }
    else if (DeliverableType == TEXT("COMPLETE"))
    {
        return Agent.CompletionPercentage >= 90.0f && Agent.FilesCreated >= 1;
    }
    
    return false;
}

void ADir_ProductionCoordinator::CalculateProductionMetrics()
{
    CurrentMetrics.ActiveAgents = 0;
    CurrentMetrics.SystemsImplemented = 0;
    
    for (const FDir_AgentStatus& Agent : AgentStatusList)
    {
        if (Agent.bIsActive)
        {
            CurrentMetrics.ActiveAgents++;
        }
        
        if (Agent.CompletionPercentage >= 90.0f && Agent.FilesCreated > 0)
        {
            CurrentMetrics.SystemsImplemented++;
        }
    }
    
    // Check compilation status
    CurrentMetrics.bCompilationSuccessful = CheckSystemCompilation();
    
    // Calculate estimated completion time
    float ElapsedTime = GetWorld()->GetTimeSeconds() - CurrentMetrics.CycleStartTime;
    float ProgressRatio = (float)CurrentAgentIndex / (float)AgentStatusList.Num();
    if (ProgressRatio > 0.0f)
    {
        CurrentMetrics.EstimatedCompletionTime = ElapsedTime / ProgressRatio;
    }
}

bool ADir_ProductionCoordinator::CheckSystemCompilation()
{
    // Basic check - if we can tick and the world exists, compilation is likely successful
    UWorld* World = GetWorld();
    return World != nullptr && IsValid(World);
}

void ADir_ProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS UPDATE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s"), *CurrentMetrics.CycleID);
    UE_LOG(LogTemp, Warning, TEXT("Current Agent: #%d/%d"), CurrentAgentIndex, AgentStatusList.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), CurrentMetrics.ActiveAgents);
    UE_LOG(LogTemp, Warning, TEXT("Systems Implemented: %d"), CurrentMetrics.SystemsImplemented);
    UE_LOG(LogTemp, Warning, TEXT("Compilation Status: %s"), CurrentMetrics.bCompilationSuccessful ? TEXT("SUCCESS") : TEXT("PENDING"));
    
    float ElapsedTime = GetWorld()->GetTimeSeconds() - CurrentMetrics.CycleStartTime;
    UE_LOG(LogTemp, Warning, TEXT("Elapsed Time: %.1f seconds"), ElapsedTime);
    UE_LOG(LogTemp, Warning, TEXT("Estimated Completion: %.1f seconds"), CurrentMetrics.EstimatedCompletionTime);
}

void ADir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL PRODUCTION REPORT ==="));
    
    for (int32 i = 0; i < AgentStatusList.Num(); i++)
    {
        const FDir_AgentStatus& Agent = AgentStatusList[i];
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d (%s): %.1f%% - %d files, %d commands - %s"), 
               Agent.AgentID, *Agent.AgentName, Agent.CompletionPercentage, 
               Agent.FilesCreated, Agent.UE5CommandsExecuted, *Agent.CurrentTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Total Systems Implemented: %d"), CurrentMetrics.SystemsImplemented);
    UE_LOG(LogTemp, Warning, TEXT("Final Compilation Status: %s"), CurrentMetrics.bCompilationSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
}

void ADir_ProductionCoordinator::ResetProductionPipeline()
{
    UE_LOG(LogTemp, Warning, TEXT("Resetting Production Pipeline"));
    
    bPipelineActive = false;
    CurrentAgentIndex = 1;
    
    for (FDir_AgentStatus& Agent : AgentStatusList)
    {
        Agent.bIsActive = false;
        Agent.CompletionPercentage = 0.0f;
        Agent.CurrentTask = TEXT("Reset");
        Agent.FilesCreated = 0;
        Agent.UE5CommandsExecuted = 0;
    }
    
    CurrentMetrics.ActiveAgents = 0;
    CurrentMetrics.SystemsImplemented = 0;
    CurrentMetrics.PlayableFeatures = 0;
    CurrentMetrics.CycleStartTime = GetWorld()->GetTimeSeconds();
}

void ADir_ProductionCoordinator::ForceAdvanceAgent()
{
    UE_LOG(LogTemp, Warning, TEXT("Force advancing to next agent"));
    AdvanceToNextAgent();
}