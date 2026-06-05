#include "QA_AgentDeliverableTracker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UQA_AgentDeliverableTracker::UQA_AgentDeliverableTracker()
{
    PrimaryComponentTick.bCanEverTick = false;
    bEnableDetailedLogging = true;
    MinimumDeliverableScore = 75.0f;
    
    // Initialize agent tracking data
    InitializeAgentTracking();
}

void UQA_AgentDeliverableTracker::InitializeAgentTracking()
{
    AgentDeliverables.Empty();
    
    // Initialize tracking for all 20 agents
    TArray<FString> AgentNames = {
        TEXT("Studio Director"), TEXT("Engine Architect"), TEXT("Core Systems"), TEXT("Performance Optimizer"),
        TEXT("Procedural World"), TEXT("Environment Artist"), TEXT("Architecture Interior"), TEXT("Lighting Atmosphere"),
        TEXT("Character Artist"), TEXT("Animation"), TEXT("NPC Behavior"), TEXT("Combat Enemy AI"),
        TEXT("Crowd Traffic"), TEXT("Quest Mission"), TEXT("Narrative Dialogue"), TEXT("Audio"),
        TEXT("VFX"), TEXT("QA Testing"), TEXT("Integration Build"), TEXT("Build Manager")
    };
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FQA_AgentDeliverableData AgentData;
        AgentData.AgentNumber = i + 1;
        AgentData.AgentName = AgentNames[i];
        AgentData.DeliverableScore = 0.0f;
        AgentData.CppFilesCreated = 0;
        AgentData.HeaderFilesCreated = 0;
        AgentData.UE5CommandsExecuted = 0;
        AgentData.AssetsCreated = 0;
        AgentData.LastCycleActivity = TEXT("Not tracked");
        AgentData.DeliverableStatus = EQA_DeliverableStatus::NotStarted;
        
        AgentDeliverables.Add(AgentData);
    }
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA_AgentDeliverableTracker: Initialized tracking for %d agents"), AgentNames.Num());
    }
}

bool UQA_AgentDeliverableTracker::TrackAgentDeliverable(int32 AgentNumber, const FString& DeliverableType, const FString& Description)
{
    if (AgentNumber < 1 || AgentNumber > AgentDeliverables.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("QA_AgentDeliverableTracker: Invalid agent number %d"), AgentNumber);
        return false;
    }
    
    FQA_AgentDeliverableData& AgentData = AgentDeliverables[AgentNumber - 1];
    
    // Update deliverable counts based on type
    if (DeliverableType.Contains(TEXT("cpp")) || DeliverableType.Contains(TEXT("CPP")))
    {
        AgentData.CppFilesCreated++;
        AgentData.DeliverableScore += 15.0f; // CPP files are high value
    }
    else if (DeliverableType.Contains(TEXT("h")) || DeliverableType.Contains(TEXT("header")))
    {
        AgentData.HeaderFilesCreated++;
        AgentData.DeliverableScore += 10.0f; // Header files are medium value
    }
    else if (DeliverableType.Contains(TEXT("ue5")) || DeliverableType.Contains(TEXT("command")))
    {
        AgentData.UE5CommandsExecuted++;
        AgentData.DeliverableScore += 12.0f; // UE5 commands are high value
    }
    else if (DeliverableType.Contains(TEXT("asset")) || DeliverableType.Contains(TEXT("blueprint")))
    {
        AgentData.AssetsCreated++;
        AgentData.DeliverableScore += 8.0f; // Assets are medium value
    }
    
    AgentData.LastCycleActivity = Description;
    
    // Update status based on score
    if (AgentData.DeliverableScore >= MinimumDeliverableScore)
    {
        AgentData.DeliverableStatus = EQA_DeliverableStatus::Completed;
    }
    else if (AgentData.DeliverableScore >= MinimumDeliverableScore * 0.5f)
    {
        AgentData.DeliverableStatus = EQA_DeliverableStatus::InProgress;
    }
    else if (AgentData.DeliverableScore > 0.0f)
    {
        AgentData.DeliverableStatus = EQA_DeliverableStatus::Started;
    }
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("QA_AgentDeliverableTracker: Agent %d (%s) - %s: %s (Score: %.1f)"), 
            AgentNumber, *AgentData.AgentName, *DeliverableType, *Description, AgentData.DeliverableScore);
    }
    
    return true;
}

FQA_AgentDeliverableData UQA_AgentDeliverableTracker::GetAgentDeliverables(int32 AgentNumber) const
{
    if (AgentNumber < 1 || AgentNumber > AgentDeliverables.Num())
    {
        FQA_AgentDeliverableData EmptyData;
        EmptyData.AgentNumber = -1;
        EmptyData.AgentName = TEXT("Invalid Agent");
        return EmptyData;
    }
    
    return AgentDeliverables[AgentNumber - 1];
}

TArray<FQA_AgentDeliverableData> UQA_AgentDeliverableTracker::GetAllAgentDeliverables() const
{
    return AgentDeliverables;
}

void UQA_AgentDeliverableTracker::GenerateDeliverableReport()
{
    FString ReportContent = TEXT("=== AGENT DELIVERABLE TRACKING REPORT ===\n\n");
    
    int32 CompletedAgents = 0;
    int32 InProgressAgents = 0;
    int32 StartedAgents = 0;
    int32 NotStartedAgents = 0;
    
    float TotalScore = 0.0f;
    int32 TotalCppFiles = 0;
    int32 TotalHeaderFiles = 0;
    int32 TotalUE5Commands = 0;
    int32 TotalAssets = 0;
    
    for (const FQA_AgentDeliverableData& AgentData : AgentDeliverables)
    {
        ReportContent += FString::Printf(TEXT("Agent #%02d - %s\n"), AgentData.AgentNumber, *AgentData.AgentName);
        ReportContent += FString::Printf(TEXT("  Score: %.1f/%.1f\n"), AgentData.DeliverableScore, MinimumDeliverableScore);
        ReportContent += FString::Printf(TEXT("  Status: %s\n"), 
            AgentData.DeliverableStatus == EQA_DeliverableStatus::Completed ? TEXT("COMPLETED") :
            AgentData.DeliverableStatus == EQA_DeliverableStatus::InProgress ? TEXT("IN PROGRESS") :
            AgentData.DeliverableStatus == EQA_DeliverableStatus::Started ? TEXT("STARTED") : TEXT("NOT STARTED"));
        ReportContent += FString::Printf(TEXT("  CPP Files: %d | Headers: %d | UE5 Commands: %d | Assets: %d\n"), 
            AgentData.CppFilesCreated, AgentData.HeaderFilesCreated, AgentData.UE5CommandsExecuted, AgentData.AssetsCreated);
        ReportContent += FString::Printf(TEXT("  Last Activity: %s\n\n"), *AgentData.LastCycleActivity);
        
        // Update counters
        switch (AgentData.DeliverableStatus)
        {
            case EQA_DeliverableStatus::Completed:
                CompletedAgents++;
                break;
            case EQA_DeliverableStatus::InProgress:
                InProgressAgents++;
                break;
            case EQA_DeliverableStatus::Started:
                StartedAgents++;
                break;
            case EQA_DeliverableStatus::NotStarted:
                NotStartedAgents++;
                break;
        }
        
        TotalScore += AgentData.DeliverableScore;
        TotalCppFiles += AgentData.CppFilesCreated;
        TotalHeaderFiles += AgentData.HeaderFilesCreated;
        TotalUE5Commands += AgentData.UE5CommandsExecuted;
        TotalAssets += AgentData.AssetsCreated;
    }
    
    ReportContent += FString::Printf(TEXT("=== SUMMARY ===\n"));
    ReportContent += FString::Printf(TEXT("Total Agents: %d\n"), AgentDeliverables.Num());
    ReportContent += FString::Printf(TEXT("Completed: %d | In Progress: %d | Started: %d | Not Started: %d\n"), 
        CompletedAgents, InProgressAgents, StartedAgents, NotStartedAgents);
    ReportContent += FString::Printf(TEXT("Average Score: %.1f/%.1f\n"), TotalScore / AgentDeliverables.Num(), MinimumDeliverableScore);
    ReportContent += FString::Printf(TEXT("Total Deliverables: CPP=%d, Headers=%d, UE5Commands=%d, Assets=%d\n"), 
        TotalCppFiles, TotalHeaderFiles, TotalUE5Commands, TotalAssets);
    
    float CompletionPercentage = (float)CompletedAgents / AgentDeliverables.Num() * 100.0f;
    ReportContent += FString::Printf(TEXT("Overall Completion: %.1f%%\n"), CompletionPercentage);
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *ReportContent);
}

TArray<int32> UQA_AgentDeliverableTracker::GetUnderperformingAgents() const
{
    TArray<int32> UnderperformingAgents;
    
    for (const FQA_AgentDeliverableData& AgentData : AgentDeliverables)
    {
        if (AgentData.DeliverableScore < MinimumDeliverableScore * 0.5f)
        {
            UnderperformingAgents.Add(AgentData.AgentNumber);
        }
    }
    
    return UnderperformingAgents;
}

void UQA_AgentDeliverableTracker::ResetAgentTracking(int32 AgentNumber)
{
    if (AgentNumber < 1 || AgentNumber > AgentDeliverables.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("QA_AgentDeliverableTracker: Cannot reset invalid agent number %d"), AgentNumber);
        return;
    }
    
    FQA_AgentDeliverableData& AgentData = AgentDeliverables[AgentNumber - 1];
    AgentData.DeliverableScore = 0.0f;
    AgentData.CppFilesCreated = 0;
    AgentData.HeaderFilesCreated = 0;
    AgentData.UE5CommandsExecuted = 0;
    AgentData.AssetsCreated = 0;
    AgentData.LastCycleActivity = TEXT("Reset");
    AgentData.DeliverableStatus = EQA_DeliverableStatus::NotStarted;
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA_AgentDeliverableTracker: Reset tracking for Agent %d (%s)"), 
            AgentNumber, *AgentData.AgentName);
    }
}

bool UQA_AgentDeliverableTracker::ValidateMinimumDeliverables()
{
    bool bAllAgentsMeetMinimum = true;
    
    for (const FQA_AgentDeliverableData& AgentData : AgentDeliverables)
    {
        if (AgentData.DeliverableScore < MinimumDeliverableScore)
        {
            bAllAgentsMeetMinimum = false;
            
            if (bEnableDetailedLogging)
            {
                UE_LOG(LogTemp, Warning, TEXT("QA_AgentDeliverableTracker: Agent %d (%s) below minimum score: %.1f/%.1f"), 
                    AgentData.AgentNumber, *AgentData.AgentName, AgentData.DeliverableScore, MinimumDeliverableScore);
            }
        }
    }
    
    return bAllAgentsMeetMinimum;
}