#include "StudioDirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UStudioDirectorSystem::UStudioDirectorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    CurrentActiveAgent = 1;
    bProductionActive = false;
    DailyBudgetUsed = 0.0f;
    DailyBudgetLimit = 100.0f;
    CurrentCycleID = TEXT("");
    
    InitializeAgentChain();
}

void UStudioDirectorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director System initialized"));
    LogAgentActivity(TEXT("Studio Director System online - Ready for production"));
}

void UStudioDirectorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bProductionActive)
    {
        UpdateCycleMetrics();
        ValidateAgentChain();
    }
}

void UStudioDirectorSystem::InitializeAgentChain()
{
    AgentChain.Empty();
    
    // Initialize all 19 agents in the production pipeline
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
        FDir_AgentInfo NewAgent;
        NewAgent.AgentID = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CurrentTask = TEXT("Waiting for cycle start");
        NewAgent.CompletionPercentage = 0.0f;
        NewAgent.LastUpdate = FDateTime::Now();
        
        AgentChain.Add(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent chain initialized with %d agents"), AgentChain.Num());
}

void UStudioDirectorSystem::StartProductionCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    bProductionActive = true;
    CurrentActiveAgent = 1;
    
    // Initialize cycle metrics
    CurrentCycleMetrics = FDir_CycleMetrics();
    CurrentCycleMetrics.CycleNumber = CycleHistory.Num() + 1;
    
    // Reset all agents to idle
    for (FDir_AgentInfo& Agent : AgentChain)
    {
        Agent.Status = EDir_AgentStatus::Idle;
        Agent.CompletionPercentage = 0.0f;
        Agent.LastUpdate = FDateTime::Now();
    }
    
    // Activate first agent (Studio Director)
    if (AgentChain.IsValidIndex(0))
    {
        AgentChain[0].Status = EDir_AgentStatus::Working;
        AgentChain[0].CurrentTask = TEXT("Coordinating cycle start");
    }
    
    LogAgentActivity(FString::Printf(TEXT("Production cycle %s started"), *CycleID));
}

void UStudioDirectorSystem::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& Task, float Completion)
{
    int32 AgentIndex = AgentID - 1;
    if (!AgentChain.IsValidIndex(AgentIndex))
    {
        return;
    }
    
    FDir_AgentInfo& Agent = AgentChain[AgentIndex];
    Agent.Status = NewStatus;
    Agent.CurrentTask = Task;
    Agent.CompletionPercentage = FMath::Clamp(Completion, 0.0f, 100.0f);
    Agent.LastUpdate = FDateTime::Now();
    
    LogAgentActivity(FString::Printf(TEXT("Agent %d (%s): %s - %.1f%% complete"), 
        AgentID, *Agent.AgentName, *Task, Completion));
    
    // If agent completed, advance to next
    if (NewStatus == EDir_AgentStatus::Completed && Completion >= 100.0f)
    {
        AdvanceToNextAgent();
    }
}

void UStudioDirectorSystem::AdvanceToNextAgent()
{
    if (CurrentActiveAgent < AgentChain.Num())
    {
        CurrentActiveAgent++;
        
        if (AgentChain.IsValidIndex(CurrentActiveAgent - 1))
        {
            AgentChain[CurrentActiveAgent - 1].Status = EDir_AgentStatus::Working;
            AgentChain[CurrentActiveAgent - 1].CurrentTask = TEXT("Starting work");
            
            LogAgentActivity(FString::Printf(TEXT("Advanced to Agent %d (%s)"), 
                CurrentActiveAgent, *AgentChain[CurrentActiveAgent - 1].AgentName));
        }
    }
    else
    {
        // All agents completed - finish cycle
        CompleteCycle();
    }
}

void UStudioDirectorSystem::CompleteCycle()
{
    bProductionActive = false;
    CurrentCycleMetrics.bCycleSuccess = true;
    CurrentCycleMetrics.TotalExecutionTime = FDateTime::Now().GetTicks() - CycleHistory.Num() * 1000000; // Rough estimate
    
    CycleHistory.Add(CurrentCycleMetrics);
    
    LogAgentActivity(FString::Printf(TEXT("Cycle %s completed successfully"), *CurrentCycleID));
    
    // Log cycle summary
    UE_LOG(LogTemp, Warning, TEXT("=== CYCLE SUMMARY ==="));
    UE_LOG(LogTemp, Warning, TEXT("Files Created: %d"), CurrentCycleMetrics.FilesCreated);
    UE_LOG(LogTemp, Warning, TEXT("UE5 Commands: %d"), CurrentCycleMetrics.UE5CommandsExecuted);
    UE_LOG(LogTemp, Warning, TEXT("Actors Spawned: %d"), CurrentCycleMetrics.ActorsSpawned);
}

void UStudioDirectorSystem::RecordFileCreation()
{
    CurrentCycleMetrics.FilesCreated++;
    LogAgentActivity(TEXT("File created"));
}

void UStudioDirectorSystem::RecordUE5Command()
{
    CurrentCycleMetrics.UE5CommandsExecuted++;
    LogAgentActivity(TEXT("UE5 command executed"));
}

void UStudioDirectorSystem::RecordActorSpawn(int32 Count)
{
    CurrentCycleMetrics.ActorsSpawned += Count;
    LogAgentActivity(FString::Printf(TEXT("%d actors spawned"), Count));
}

FDir_AgentInfo UStudioDirectorSystem::GetAgentInfo(int32 AgentID) const
{
    int32 AgentIndex = AgentID - 1;
    if (AgentChain.IsValidIndex(AgentIndex))
    {
        return AgentChain[AgentIndex];
    }
    
    return FDir_AgentInfo();
}

bool UStudioDirectorSystem::IsAgentActive(int32 AgentID) const
{
    int32 AgentIndex = AgentID - 1;
    if (AgentChain.IsValidIndex(AgentIndex))
    {
        return AgentChain[AgentIndex].Status == EDir_AgentStatus::Working;
    }
    
    return false;
}

float UStudioDirectorSystem::GetCycleProgress() const
{
    if (AgentChain.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_AgentInfo& Agent : AgentChain)
    {
        TotalProgress += Agent.CompletionPercentage;
    }
    
    return TotalProgress / AgentChain.Num();
}

int32 UStudioDirectorSystem::GetTotalActorsInScene() const
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

void UStudioDirectorSystem::EmergencyStopCycle(const FString& Reason)
{
    bProductionActive = false;
    CurrentCycleMetrics.bCycleSuccess = false;
    
    for (FDir_AgentInfo& Agent : AgentChain)
    {
        if (Agent.Status == EDir_AgentStatus::Working)
        {
            Agent.Status = EDir_AgentStatus::Failed;
            Agent.CurrentTask = FString::Printf(TEXT("Emergency stop: %s"), *Reason);
        }
    }
    
    LogAgentActivity(FString::Printf(TEXT("EMERGENCY STOP: %s"), *Reason));
    UE_LOG(LogTemp, Error, TEXT("Production cycle emergency stop: %s"), *Reason);
}

void UStudioDirectorSystem::ResetAgentChain()
{
    InitializeAgentChain();
    CurrentActiveAgent = 1;
    bProductionActive = false;
    
    LogAgentActivity(TEXT("Agent chain reset"));
}

void UStudioDirectorSystem::LogAgentActivity(const FString& Message)
{
    FString LogMessage = FString::Printf(TEXT("[DIRECTOR] %s"), *Message);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, LogMessage);
    }
}

void UStudioDirectorSystem::ValidateAgentChain()
{
    // Check for stuck agents
    FDateTime CurrentTime = FDateTime::Now();
    for (FDir_AgentInfo& Agent : AgentChain)
    {
        if (Agent.Status == EDir_AgentStatus::Working)
        {
            FTimespan TimeSinceUpdate = CurrentTime - Agent.LastUpdate;
            if (TimeSinceUpdate.GetTotalMinutes() > 5.0) // 5 minute timeout
            {
                Agent.Status = EDir_AgentStatus::Failed;
                Agent.CurrentTask = TEXT("Timeout - Agent stuck");
                
                LogAgentActivity(FString::Printf(TEXT("Agent %d timeout detected"), Agent.AgentID));
            }
        }
    }
}

void UStudioDirectorSystem::UpdateCycleMetrics()
{
    // Update real-time metrics
    if (UWorld* World = GetWorld())
    {
        CurrentCycleMetrics.ActorsSpawned = GetTotalActorsInScene();
    }
}