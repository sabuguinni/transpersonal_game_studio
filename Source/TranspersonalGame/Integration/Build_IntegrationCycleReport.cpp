#include "Build_IntegrationCycleReport.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"

void UBuild_IntegrationCycleReport::AddAgentData(const FBuild_AgentCycleData& Data)
{
    // Check if agent data already exists and update, or add new
    bool bFound = false;
    for (FBuild_AgentCycleData& ExistingData : AgentData)
    {
        if (ExistingData.AgentType == Data.AgentType)
        {
            ExistingData = Data;
            bFound = true;
            break;
        }
    }
    
    if (!bFound)
    {
        AgentData.Add(Data);
    }
    
    UpdateCycleMetrics();
}

void UBuild_IntegrationCycleReport::UpdateCycleMetrics()
{
    CycleMetrics.TotalAgentsCompleted = 0;
    CycleMetrics.TotalFilesCreated = 0;
    CycleMetrics.TotalUE5Commands = 0;
    CycleMetrics.TotalExecutionTime = 0.0f;
    
    for (const FBuild_AgentCycleData& Data : AgentData)
    {
        if (Data.Status == EBuild_CycleStatus::Completed)
        {
            CycleMetrics.TotalAgentsCompleted++;
        }
        
        CycleMetrics.TotalFilesCreated += Data.FilesCreated;
        CycleMetrics.TotalUE5Commands += Data.UE5CommandsExecuted;
        CycleMetrics.TotalExecutionTime += Data.ExecutionTimeSeconds;
    }
    
    // Update scene metrics if we have a valid world
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CycleMetrics.TotalActorsInScene = AllActors.Num();
        
        // Count dinosaurs
        int32 DinosaurCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("Dino"), ESearchCase::IgnoreCase))
            {
                DinosaurCount++;
            }
        }
        CycleMetrics.TotalDinosaursInScene = DinosaurCount;
    }
    
    // Check validation status
    CycleMetrics.bAllValidationsPassed = (ValidationFailures.Num() == 0);
    CycleMetrics.bCompilationSuccessful = (CompilationErrors.Num() == 0);
}

void UBuild_IntegrationCycleReport::FinalizeCycle(EBuild_CycleStatus FinalStatus)
{
    OverallStatus = FinalStatus;
    CycleEndTime = FDateTime::Now();
    UpdateCycleMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Cycle %s finalized with status: %s"), 
           *CycleID, 
           *UEnum::GetValueAsString(FinalStatus));
}

float UBuild_IntegrationCycleReport::GetCycleDurationSeconds() const
{
    if (CycleEndTime > CycleStartTime)
    {
        return (CycleEndTime - CycleStartTime).GetTotalSeconds();
    }
    else if (CycleStartTime.GetTicks() > 0)
    {
        return (FDateTime::Now() - CycleStartTime).GetTotalSeconds();
    }
    return 0.0f;
}

bool UBuild_IntegrationCycleReport::IsSuccessfulCycle() const
{
    return (OverallStatus == EBuild_CycleStatus::Completed) && 
           CycleMetrics.bAllValidationsPassed && 
           CycleMetrics.bCompilationSuccessful &&
           (CycleMetrics.TotalAgentsCompleted >= 15); // At least 15 of 19 agents completed
}

FString UBuild_IntegrationCycleReport::GenerateSummaryReport() const
{
    FString Report;
    
    Report += FString::Printf(TEXT("=== INTEGRATION CYCLE REPORT ===\n"));
    Report += FString::Printf(TEXT("Cycle ID: %s\n"), *CycleID);
    Report += FString::Printf(TEXT("Duration: %.1f seconds\n"), GetCycleDurationSeconds());
    Report += FString::Printf(TEXT("Status: %s\n"), *UEnum::GetValueAsString(OverallStatus));
    Report += FString::Printf(TEXT("\n"));
    
    Report += FString::Printf(TEXT("=== METRICS ===\n"));
    Report += FString::Printf(TEXT("Agents Completed: %d/19\n"), CycleMetrics.TotalAgentsCompleted);
    Report += FString::Printf(TEXT("Files Created: %d\n"), CycleMetrics.TotalFilesCreated);
    Report += FString::Printf(TEXT("UE5 Commands: %d\n"), CycleMetrics.TotalUE5Commands);
    Report += FString::Printf(TEXT("Total Execution Time: %.1f seconds\n"), CycleMetrics.TotalExecutionTime);
    Report += FString::Printf(TEXT("Actors in Scene: %d\n"), CycleMetrics.TotalActorsInScene);
    Report += FString::Printf(TEXT("Dinosaurs in Scene: %d\n"), CycleMetrics.TotalDinosaursInScene);
    Report += FString::Printf(TEXT("Compilation: %s\n"), CycleMetrics.bCompilationSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    Report += FString::Printf(TEXT("Validations: %s\n"), CycleMetrics.bAllValidationsPassed ? TEXT("PASSED") : TEXT("FAILED"));
    Report += FString::Printf(TEXT("\n"));
    
    if (CompilationErrors.Num() > 0)
    {
        Report += FString::Printf(TEXT("=== COMPILATION ERRORS ===\n"));
        for (const FString& Error : CompilationErrors)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Error);
        }
        Report += FString::Printf(TEXT("\n"));
    }
    
    if (ValidationFailures.Num() > 0)
    {
        Report += FString::Printf(TEXT("=== VALIDATION FAILURES ===\n"));
        for (const FString& Failure : ValidationFailures)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Failure);
        }
        Report += FString::Printf(TEXT("\n"));
    }
    
    if (IntegrationIssues.Num() > 0)
    {
        Report += FString::Printf(TEXT("=== INTEGRATION ISSUES ===\n"));
        for (const FString& Issue : IntegrationIssues)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Issue);
        }
        Report += FString::Printf(TEXT("\n"));
    }
    
    // Agent status breakdown
    Report += FString::Printf(TEXT("=== AGENT STATUS ===\n"));
    for (const FBuild_AgentCycleData& Data : AgentData)
    {
        FString AgentName = UEnum::GetValueAsString(Data.AgentType);
        FString StatusName = UEnum::GetValueAsString(Data.Status);
        Report += FString::Printf(TEXT("%s: %s (%d files, %d commands, %.1fs)\n"), 
                                 *AgentName, *StatusName, Data.FilesCreated, Data.UE5CommandsExecuted, Data.ExecutionTimeSeconds);
    }
    
    if (!FinalNotes.IsEmpty())
    {
        Report += FString::Printf(TEXT("\n=== FINAL NOTES ===\n"));
        Report += FinalNotes;
        Report += FString::Printf(TEXT("\n"));
    }
    
    Report += FString::Printf(TEXT("\n=== OVERALL RESULT ===\n"));
    Report += FString::Printf(TEXT("Cycle Success: %s\n"), IsSuccessfulCycle() ? TEXT("YES") : TEXT("NO"));
    
    return Report;
}