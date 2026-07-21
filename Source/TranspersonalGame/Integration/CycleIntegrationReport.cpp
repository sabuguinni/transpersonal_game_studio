#include "CycleIntegrationReport.h"
#include "Engine/Engine.h"

void UCycleIntegrationReport::InitializeCycle(const FString& InCycleID)
{
    CycleID = InCycleID;
    CycleStartTime = FDateTime::Now();
    CycleStatus = EIntegrationCycleStatus::InProgress;
    
    AgentReports.Empty();
    CriticalIssues.Empty();
    Recommendations.Empty();
    
    // Reset metrics
    Metrics = FCycleIntegrationMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Initialized integration cycle: %s"), *CycleID);
}

void UCycleIntegrationReport::AddAgentReport(const FAgentCycleReport& AgentReport)
{
    AgentReports.Add(AgentReport);
    
    UE_LOG(LogTemp, Log, TEXT("Added report for Agent #%d (%s): %d files, %d commands, %.2fs"), 
        AgentReport.AgentID, 
        *AgentReport.AgentName,
        AgentReport.FilesCreated,
        AgentReport.UE5CommandsExecuted,
        AgentReport.ExecutionTimeSeconds);
}

void UCycleIntegrationReport::FinalizeCycle()
{
    CycleEndTime = FDateTime::Now();
    
    CalculateMetrics();
    AnalyzeCriticalIssues();
    GenerateRecommendations();
    
    // Determine final status
    if (Metrics.FailedAgents == 0 && CriticalIssues.Num() == 0)
    {
        CycleStatus = EIntegrationCycleStatus::Completed;
    }
    else if (Metrics.FailedAgents > Metrics.CompletedAgents / 2)
    {
        CycleStatus = EIntegrationCycleStatus::Failed;
    }
    else
    {
        CycleStatus = EIntegrationCycleStatus::RequiresAttention;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Finalized cycle %s with status: %s"), 
        *CycleID, 
        *UEnum::GetValueAsString(CycleStatus));
}

FString UCycleIntegrationReport::GenerateReportSummary() const
{
    FString Summary;
    
    Summary += FString::Printf(TEXT("=== CYCLE INTEGRATION REPORT ===\n"));
    Summary += FString::Printf(TEXT("Cycle ID: %s\n"), *CycleID);
    Summary += FString::Printf(TEXT("Status: %s\n"), *UEnum::GetValueAsString(CycleStatus));
    Summary += FString::Printf(TEXT("Duration: %.2f minutes\n"), (CycleEndTime - CycleStartTime).GetTotalMinutes());
    Summary += FString::Printf(TEXT("\n"));
    
    Summary += FString::Printf(TEXT("=== METRICS ===\n"));
    Summary += FString::Printf(TEXT("Agents: %d total, %d completed, %d failed\n"), 
        Metrics.TotalAgents, Metrics.CompletedAgents, Metrics.FailedAgents);
    Summary += FString::Printf(TEXT("Files Created: %d\n"), Metrics.TotalFilesCreated);
    Summary += FString::Printf(TEXT("UE5 Commands: %d\n"), Metrics.TotalUE5Commands);
    Summary += FString::Printf(TEXT("Compilation Errors: %d\n"), Metrics.CompilationErrors);
    Summary += FString::Printf(TEXT("Success Rate: %.1f%%\n"), GetCycleSuccessRate());
    Summary += FString::Printf(TEXT("Average Agent Time: %.2f seconds\n"), Metrics.AverageAgentTime);
    Summary += FString::Printf(TEXT("\n"));
    
    if (CriticalIssues.Num() > 0)
    {
        Summary += FString::Printf(TEXT("=== CRITICAL ISSUES ===\n"));
        for (const FString& Issue : CriticalIssues)
        {
            Summary += FString::Printf(TEXT("- %s\n"), *Issue);
        }
        Summary += FString::Printf(TEXT("\n"));
    }
    
    if (Recommendations.Num() > 0)
    {
        Summary += FString::Printf(TEXT("=== RECOMMENDATIONS ===\n"));
        for (const FString& Recommendation : Recommendations)
        {
            Summary += FString::Printf(TEXT("- %s\n"), *Recommendation);
        }
        Summary += FString::Printf(TEXT("\n"));
    }
    
    Summary += FString::Printf(TEXT("=== AGENT DETAILS ===\n"));
    for (const FAgentCycleReport& Report : AgentReports)
    {
        Summary += FString::Printf(TEXT("Agent #%d (%s): %d files, %d commands, %.2fs"), 
            Report.AgentID, *Report.AgentName, Report.FilesCreated, 
            Report.UE5CommandsExecuted, Report.ExecutionTimeSeconds);
        
        if (!Report.bCompilationSuccess)
        {
            Summary += FString::Printf(TEXT(" [COMPILATION FAILED]"));
        }
        
        if (Report.ErrorMessages.Num() > 0)
        {
            Summary += FString::Printf(TEXT(" [%d ERRORS]"), Report.ErrorMessages.Num());
        }
        
        Summary += FString::Printf(TEXT("\n"));
    }
    
    return Summary;
}

bool UCycleIntegrationReport::HasCriticalIssues() const
{
    return CriticalIssues.Num() > 0 || Metrics.FailedAgents > 0;
}

float UCycleIntegrationReport::GetCycleSuccessRate() const
{
    if (Metrics.TotalAgents == 0)
    {
        return 0.0f;
    }
    
    return (float)Metrics.CompletedAgents / (float)Metrics.TotalAgents * 100.0f;
}

TArray<FString> UCycleIntegrationReport::GetTopIssues() const
{
    TArray<FString> TopIssues;
    
    // Collect all error messages from agents
    TMap<FString, int32> ErrorCounts;
    
    for (const FAgentCycleReport& Report : AgentReports)
    {
        for (const FString& Error : Report.ErrorMessages)
        {
            if (ErrorCounts.Contains(Error))
            {
                ErrorCounts[Error]++;
            }
            else
            {
                ErrorCounts.Add(Error, 1);
            }
        }
    }
    
    // Sort by frequency
    ErrorCounts.ValueSort([](const int32& A, const int32& B) {
        return A > B;
    });
    
    // Get top 5 issues
    int32 Count = 0;
    for (const auto& ErrorPair : ErrorCounts)
    {
        if (Count >= 5) break;
        
        TopIssues.Add(FString::Printf(TEXT("%s (x%d)"), *ErrorPair.Key, ErrorPair.Value));
        Count++;
    }
    
    return TopIssues;
}

void UCycleIntegrationReport::CalculateMetrics()
{
    Metrics.TotalAgents = AgentReports.Num();
    Metrics.CompletedAgents = 0;
    Metrics.FailedAgents = 0;
    Metrics.TotalFilesCreated = 0;
    Metrics.TotalUE5Commands = 0;
    Metrics.CompilationErrors = 0;
    Metrics.TotalExecutionTime = 0.0f;
    
    for (const FAgentCycleReport& Report : AgentReports)
    {
        if (Report.ErrorMessages.Num() == 0 && Report.bCompilationSuccess)
        {
            Metrics.CompletedAgents++;
        }
        else
        {
            Metrics.FailedAgents++;
        }
        
        Metrics.TotalFilesCreated += Report.FilesCreated;
        Metrics.TotalUE5Commands += Report.UE5CommandsExecuted;
        Metrics.TotalExecutionTime += Report.ExecutionTimeSeconds;
        
        if (!Report.bCompilationSuccess)
        {
            Metrics.CompilationErrors++;
        }
    }
    
    if (Metrics.TotalAgents > 0)
    {
        Metrics.AverageAgentTime = Metrics.TotalExecutionTime / Metrics.TotalAgents;
    }
}

void UCycleIntegrationReport::AnalyzeCriticalIssues()
{
    CriticalIssues.Empty();
    
    // Check for high failure rate
    if (Metrics.TotalAgents > 0)
    {
        float FailureRate = (float)Metrics.FailedAgents / (float)Metrics.TotalAgents;
        if (FailureRate > 0.3f) // More than 30% failure
        {
            CriticalIssues.Add(FString::Printf(TEXT("High failure rate: %.1f%% of agents failed"), FailureRate * 100.0f));
        }
    }
    
    // Check for compilation issues
    if (Metrics.CompilationErrors > 0)
    {
        CriticalIssues.Add(FString::Printf(TEXT("Compilation errors detected in %d agents"), Metrics.CompilationErrors));
    }
    
    // Check for low productivity
    if (Metrics.TotalFilesCreated < Metrics.TotalAgents * 2) // Less than 2 files per agent on average
    {
        CriticalIssues.Add(TEXT("Low file creation productivity detected"));
    }
    
    // Check for UE5 command execution issues
    if (Metrics.TotalUE5Commands < Metrics.TotalAgents) // Less than 1 command per agent on average
    {
        CriticalIssues.Add(TEXT("Low UE5 command execution rate detected"));
    }
}

void UCycleIntegrationReport::GenerateRecommendations()
{
    Recommendations.Empty();
    
    // Based on critical issues, generate recommendations
    for (const FString& Issue : CriticalIssues)
    {
        if (Issue.Contains(TEXT("failure rate")))
        {
            Recommendations.Add(TEXT("Review agent configurations and bridge validation procedures"));
        }
        else if (Issue.Contains(TEXT("Compilation errors")))
        {
            Recommendations.Add(TEXT("Implement pre-compilation validation and header dependency checks"));
        }
        else if (Issue.Contains(TEXT("productivity")))
        {
            Recommendations.Add(TEXT("Increase minimum file creation requirements per agent"));
        }
        else if (Issue.Contains(TEXT("command execution")))
        {
            Recommendations.Add(TEXT("Verify UE5 bridge connectivity and command validation"));
        }
    }
    
    // General recommendations based on metrics
    if (Metrics.AverageAgentTime > 120.0f) // More than 2 minutes per agent
    {
        Recommendations.Add(TEXT("Optimize agent execution time - consider reducing complexity"));
    }
    
    if (GetCycleSuccessRate() < 80.0f)
    {
        Recommendations.Add(TEXT("Implement additional quality gates and validation steps"));
    }
}