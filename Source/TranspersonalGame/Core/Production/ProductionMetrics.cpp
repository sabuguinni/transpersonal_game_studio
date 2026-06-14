#include "ProductionMetrics.h"
#include "Engine/World.h"

void UProductionMetricsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionMetricsSubsystem initialized"));
    
    // Initialize first cycle
    StartNewCycle(TEXT("PROD_CYCLE_AUTO_20260614_003"));
}

void UProductionMetricsSubsystem::RecordAgentActivity(const FString& AgentName, int32 FilesCreated, int32 UE5Commands)
{
    // Find existing agent or create new entry
    FDir_AgentMetrics* ExistingAgent = CurrentCycle.AgentMetrics.FindByPredicate(
        [&AgentName](const FDir_AgentMetrics& Agent) 
        { 
            return Agent.AgentName == AgentName; 
        }
    );

    if (ExistingAgent)
    {
        ExistingAgent->FilesCreated += FilesCreated;
        ExistingAgent->UE5CommandsExecuted += UE5Commands;
        ExistingAgent->LastActivity = FDateTime::Now();
    }
    else
    {
        FDir_AgentMetrics NewAgent;
        NewAgent.AgentName = AgentName;
        NewAgent.FilesCreated = FilesCreated;
        NewAgent.UE5CommandsExecuted = UE5Commands;
        NewAgent.LastActivity = FDateTime::Now();
        CurrentCycle.AgentMetrics.Add(NewAgent);
    }

    CalculateAgentScores();
    
    UE_LOG(LogTemp, Log, TEXT("Agent %s activity recorded: %d files, %d UE5 commands"), 
           *AgentName, FilesCreated, UE5Commands);
}

void UProductionMetricsSubsystem::StartNewCycle(const FString& CycleID)
{
    // Archive current cycle if it exists
    if (!CurrentCycle.CycleID.IsEmpty())
    {
        CycleHistory.Add(CurrentCycle);
        
        // Keep only last 10 cycles
        if (CycleHistory.Num() > 10)
        {
            CycleHistory.RemoveAt(0);
        }
    }

    // Initialize new cycle
    CurrentCycle = FDir_CycleMetrics();
    CurrentCycle.CycleID = CycleID;
    
    UE_LOG(LogTemp, Warning, TEXT("Started new production cycle: %s"), *CycleID);
}

void UProductionMetricsSubsystem::UpdateLevelMetrics(int32 TotalActors, int32 Dinosaurs, int32 Characters)
{
    CurrentCycle.TotalActorsInLevel = TotalActors;
    CurrentCycle.DinosaurCount = Dinosaurs;
    CurrentCycle.CharacterCount = Characters;
    
    UE_LOG(LogTemp, Log, TEXT("Level metrics updated: %d total actors, %d dinosaurs, %d characters"), 
           TotalActors, Dinosaurs, Characters);
}

FDir_CycleMetrics UProductionMetricsSubsystem::GetCurrentCycleMetrics() const
{
    return CurrentCycle;
}

TArray<FDir_AgentMetrics> UProductionMetricsSubsystem::GetTopPerformingAgents(int32 Count) const
{
    TArray<FDir_AgentMetrics> SortedAgents = CurrentCycle.AgentMetrics;
    
    // Sort by production score descending
    SortedAgents.Sort([](const FDir_AgentMetrics& A, const FDir_AgentMetrics& B)
    {
        return A.ProductionScore > B.ProductionScore;
    });
    
    // Return top N agents
    if (SortedAgents.Num() > Count)
    {
        SortedAgents.SetNum(Count);
    }
    
    return SortedAgents;
}

float UProductionMetricsSubsystem::CalculateOverallProductionScore() const
{
    if (CurrentCycle.AgentMetrics.Num() == 0)
    {
        return 0.0f;
    }

    float TotalScore = 0.0f;
    for (const FDir_AgentMetrics& Agent : CurrentCycle.AgentMetrics)
    {
        TotalScore += Agent.ProductionScore;
    }

    return TotalScore / CurrentCycle.AgentMetrics.Num();
}

void UProductionMetricsSubsystem::CalculateAgentScores()
{
    for (FDir_AgentMetrics& Agent : CurrentCycle.AgentMetrics)
    {
        // Score based on files created (40%) + UE5 commands (40%) + recency (20%)
        float FileScore = FMath::Min(Agent.FilesCreated * 10.0f, 40.0f);
        float CommandScore = FMath::Min(Agent.UE5CommandsExecuted * 10.0f, 40.0f);
        
        // Recency bonus (last 24 hours = full 20 points)
        FTimespan TimeSinceActivity = FDateTime::Now() - Agent.LastActivity;
        float RecencyScore = FMath::Max(0.0f, 20.0f - (TimeSinceActivity.GetTotalHours() * 0.83f));
        
        Agent.ProductionScore = FileScore + CommandScore + RecencyScore;
    }
}