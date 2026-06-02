#include "QA_AgentPerformanceScorecard.h"
#include "Engine/Engine.h"

UQA_AgentPerformanceScorecard::UQA_AgentPerformanceScorecard()
{
    TotalAgents = 20;
    OverallScore = 0.0f;
}

void UQA_AgentPerformanceScorecard::InitializeScorecard()
{
    AgentMetrics.Empty();
    
    // Initialize metrics for all 20 agents
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
        TEXT("Integration & Build Agent"),
        TEXT("Build & Deployment Agent")
    };
    
    for (int32 i = 0; i < TotalAgents; i++)
    {
        FQA_AgentMetrics Metrics;
        Metrics.AgentNumber = i + 1;
        Metrics.AgentName = AgentNames[i];
        AgentMetrics.Add(Metrics);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA Scorecard initialized for %d agents"), TotalAgents);
}

void UQA_AgentPerformanceScorecard::UpdateAgentMetrics(int32 AgentNumber, const FQA_AgentMetrics& Metrics)
{
    if (AgentNumber < 1 || AgentNumber > TotalAgents)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid agent number: %d"), AgentNumber);
        return;
    }
    
    int32 Index = AgentNumber - 1;
    if (AgentMetrics.IsValidIndex(Index))
    {
        AgentMetrics[Index] = Metrics;
        AgentMetrics[Index].Status = DetermineAgentStatus(Metrics);
        
        UE_LOG(LogTemp, Warning, TEXT("Updated metrics for Agent #%d: %s"), 
               AgentNumber, *Metrics.AgentName);
    }
    
    CalculateOverallScore();
}

FQA_AgentMetrics UQA_AgentPerformanceScorecard::GetAgentMetrics(int32 AgentNumber) const
{
    if (AgentNumber < 1 || AgentNumber > TotalAgents)
    {
        return FQA_AgentMetrics();
    }
    
    int32 Index = AgentNumber - 1;
    if (AgentMetrics.IsValidIndex(Index))
    {
        return AgentMetrics[Index];
    }
    
    return FQA_AgentMetrics();
}

void UQA_AgentPerformanceScorecard::GeneratePerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT PERFORMANCE SCORECARD ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Score: %.1f%%"), OverallScore);
    UE_LOG(LogTemp, Warning, TEXT("Total C++ Files: %d"), GetTotalCppFiles());
    UE_LOG(LogTemp, Warning, TEXT("Total UE5 Commands: %d"), GetTotalUE5Commands());
    UE_LOG(LogTemp, Warning, TEXT("Deliverable Success Rate: %.1f%%"), GetDeliverableSuccessRate());
    
    // Report by status
    int32 ExcellentCount = 0, GoodCount = 0, WarningCount = 0, CriticalCount = 0, BlockedCount = 0;
    
    for (const FQA_AgentMetrics& Metrics : AgentMetrics)
    {
        switch (Metrics.Status)
        {
            case EQA_AgentStatus::Excellent: ExcellentCount++; break;
            case EQA_AgentStatus::Good: GoodCount++; break;
            case EQA_AgentStatus::Warning: WarningCount++; break;
            case EQA_AgentStatus::Critical: CriticalCount++; break;
            case EQA_AgentStatus::Blocked: BlockedCount++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Status Distribution:"));
    UE_LOG(LogTemp, Warning, TEXT("  Excellent: %d"), ExcellentCount);
    UE_LOG(LogTemp, Warning, TEXT("  Good: %d"), GoodCount);
    UE_LOG(LogTemp, Warning, TEXT("  Warning: %d"), WarningCount);
    UE_LOG(LogTemp, Warning, TEXT("  Critical: %d"), CriticalCount);
    UE_LOG(LogTemp, Warning, TEXT("  Blocked: %d"), BlockedCount);
    
    // Detailed agent report
    UE_LOG(LogTemp, Warning, TEXT("\n=== DETAILED AGENT METRICS ==="));
    for (const FQA_AgentMetrics& Metrics : AgentMetrics)
    {
        FString StatusStr;
        switch (Metrics.Status)
        {
            case EQA_AgentStatus::Excellent: StatusStr = TEXT("EXCELLENT"); break;
            case EQA_AgentStatus::Good: StatusStr = TEXT("GOOD"); break;
            case EQA_AgentStatus::Warning: StatusStr = TEXT("WARNING"); break;
            case EQA_AgentStatus::Critical: StatusStr = TEXT("CRITICAL"); break;
            case EQA_AgentStatus::Blocked: StatusStr = TEXT("BLOCKED"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Agent #%d %s [%s]: CPP=%d, UE5=%d, Actors=%d, Concrete=%s"), 
               Metrics.AgentNumber,
               *Metrics.AgentName,
               *StatusStr,
               Metrics.CppFilesCreated,
               Metrics.UE5CommandsExecuted,
               Metrics.ActorsSpawned,
               Metrics.bProducedConcreteDeliverables ? TEXT("YES") : TEXT("NO"));
    }
    
    // Critical agents requiring attention
    TArray<int32> CriticalAgents = GetCriticalAgents();
    if (CriticalAgents.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("\n=== AGENTS REQUIRING IMMEDIATE ATTENTION ==="));
        for (int32 AgentNum : CriticalAgents)
        {
            FQA_AgentMetrics Metrics = GetAgentMetrics(AgentNum);
            UE_LOG(LogTemp, Error, TEXT("Agent #%d %s: %s"), 
                   AgentNum, *Metrics.AgentName, *Metrics.Notes);
        }
    }
}

int32 UQA_AgentPerformanceScorecard::GetTotalCppFiles() const
{
    int32 Total = 0;
    for (const FQA_AgentMetrics& Metrics : AgentMetrics)
    {
        Total += Metrics.CppFilesCreated;
    }
    return Total;
}

int32 UQA_AgentPerformanceScorecard::GetTotalUE5Commands() const
{
    int32 Total = 0;
    for (const FQA_AgentMetrics& Metrics : AgentMetrics)
    {
        Total += Metrics.UE5CommandsExecuted;
    }
    return Total;
}

float UQA_AgentPerformanceScorecard::GetDeliverableSuccessRate() const
{
    if (AgentMetrics.Num() == 0) return 0.0f;
    
    int32 SuccessfulAgents = 0;
    for (const FQA_AgentMetrics& Metrics : AgentMetrics)
    {
        if (Metrics.bProducedConcreteDeliverables)
        {
            SuccessfulAgents++;
        }
    }
    
    return (float)SuccessfulAgents / (float)AgentMetrics.Num() * 100.0f;
}

TArray<int32> UQA_AgentPerformanceScorecard::GetCriticalAgents() const
{
    TArray<int32> CriticalAgents;
    
    for (const FQA_AgentMetrics& Metrics : AgentMetrics)
    {
        if (Metrics.Status == EQA_AgentStatus::Critical || 
            Metrics.Status == EQA_AgentStatus::Blocked)
        {
            CriticalAgents.Add(Metrics.AgentNumber);
        }
    }
    
    return CriticalAgents;
}

void UQA_AgentPerformanceScorecard::CalculateOverallScore()
{
    if (AgentMetrics.Num() == 0)
    {
        OverallScore = 0.0f;
        return;
    }
    
    float TotalScore = 0.0f;
    
    for (const FQA_AgentMetrics& Metrics : AgentMetrics)
    {
        float AgentScore = 0.0f;
        
        // Score based on deliverables
        if (Metrics.CppFilesCreated > 0) AgentScore += 25.0f;
        if (Metrics.UE5CommandsExecuted > 0) AgentScore += 25.0f;
        if (Metrics.bProducedConcreteDeliverables) AgentScore += 30.0f;
        if (Metrics.bBuildsOnExistingContent) AgentScore += 20.0f;
        
        TotalScore += AgentScore;
    }
    
    OverallScore = TotalScore / (float)AgentMetrics.Num();
}

EQA_AgentStatus UQA_AgentPerformanceScorecard::DetermineAgentStatus(const FQA_AgentMetrics& Metrics)
{
    // Blocked: No deliverables at all
    if (Metrics.CppFilesCreated == 0 && Metrics.UE5CommandsExecuted == 0 && !Metrics.bProducedConcreteDeliverables)
    {
        return EQA_AgentStatus::Blocked;
    }
    
    // Critical: Only assessment/reports, no concrete deliverables
    if (!Metrics.bProducedConcreteDeliverables)
    {
        return EQA_AgentStatus::Critical;
    }
    
    // Warning: Some deliverables but not building on existing content
    if (!Metrics.bBuildsOnExistingContent)
    {
        return EQA_AgentStatus::Warning;
    }
    
    // Good: Has deliverables and builds on existing content
    if (Metrics.CppFilesCreated > 0 && Metrics.UE5CommandsExecuted > 0)
    {
        return EQA_AgentStatus::Good;
    }
    
    // Excellent: Multiple deliverables, UE5 commands, and concrete output
    if (Metrics.CppFilesCreated >= 2 && Metrics.UE5CommandsExecuted >= 2 && Metrics.ActorsSpawned > 0)
    {
        return EQA_AgentStatus::Excellent;
    }
    
    return EQA_AgentStatus::Warning;
}