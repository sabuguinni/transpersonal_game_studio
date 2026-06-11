#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

void UProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initializing Studio Director coordination system"));
    
    InitializeAgentRegistry();
    ProductionMetrics.CurrentPhase = EDir_ProductionPhase::Prototype;
    ProductionMetrics.CurrentMilestone = "Minimum Viable Playable Prototype";
    
    UpdateProductionMetrics();
}

void UProductionCoordinator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Shutting down coordination system"));
    Super::Deinitialize();
}

void UProductionCoordinator::InitializeAgentRegistry()
{
    // Register all 19 agents in the production pipeline
    RegisterAgent(1, "Studio Director");
    RegisterAgent(2, "Engine Architect");
    RegisterAgent(3, "Core Systems Programmer");
    RegisterAgent(4, "Performance Optimizer");
    RegisterAgent(5, "Procedural World Generator");
    RegisterAgent(6, "Environment Artist");
    RegisterAgent(7, "Architecture & Interior Agent");
    RegisterAgent(8, "Lighting & Atmosphere Agent");
    RegisterAgent(9, "Character Artist Agent");
    RegisterAgent(10, "Animation Agent");
    RegisterAgent(11, "NPC Behavior Agent");
    RegisterAgent(12, "Combat & Enemy AI Agent");
    RegisterAgent(13, "Crowd & Traffic Simulation");
    RegisterAgent(14, "Quest & Mission Designer");
    RegisterAgent(15, "Narrative & Dialogue Agent");
    RegisterAgent(16, "Audio Agent");
    RegisterAgent(17, "VFX Agent");
    RegisterAgent(18, "QA & Testing Agent");
    RegisterAgent(19, "Integration & Build Agent");
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Registered %d agents"), AgentTasks.Num());
}

void UProductionCoordinator::RegisterAgent(int32 AgentID, const FString& AgentName)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.AgentName = AgentName;
    NewTask.Status = EDir_AgentStatus::Idle;
    NewTask.CurrentTask = "Awaiting assignment";
    NewTask.Progress = 0.0f;
    NewTask.LastUpdate = FDateTime::Now();
    
    AgentTasks.Add(AgentID, NewTask);
    
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Registered Agent #%d - %s"), AgentID, *AgentName);
}

void UProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus Status, const FString& CurrentTask)
{
    if (FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        Task->Status = Status;
        Task->LastUpdate = FDateTime::Now();
        
        if (!CurrentTask.IsEmpty())
        {
            Task->CurrentTask = CurrentTask;
        }
        
        // Update progress based on status
        switch (Status)
        {
            case EDir_AgentStatus::Working:
                Task->Progress = FMath::Clamp(Task->Progress + 0.1f, 0.0f, 0.9f);
                break;
            case EDir_AgentStatus::Complete:
                Task->Progress = 1.0f;
                break;
            case EDir_AgentStatus::Failed:
                Task->Progress = 0.0f;
                break;
            default:
                break;
        }
        
        UpdateProductionMetrics();
        
        UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Agent #%d (%s) status updated to %d - %s"), 
               AgentID, *Task->AgentName, (int32)Status, *CurrentTask);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent #%d not found"), AgentID);
    }
}

void UProductionCoordinator::RecordAgentDeliverable(int32 AgentID, const FString& Deliverable)
{
    if (FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        Task->LastDeliverable = Deliverable;
        Task->LastUpdate = FDateTime::Now();
        Task->Progress = FMath::Clamp(Task->Progress + 0.2f, 0.0f, 1.0f);
        
        ProductionMetrics.CompletedTasks++;
        UpdateProductionMetrics();
        
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent #%d delivered: %s"), AgentID, *Deliverable);
    }
}

FDir_AgentTask UProductionCoordinator::GetAgentStatus(int32 AgentID) const
{
    if (const FDir_AgentTask* Task = AgentTasks.Find(AgentID))
    {
        return *Task;
    }
    
    return FDir_AgentTask();
}

TArray<FDir_AgentTask> UProductionCoordinator::GetAllAgentStatuses() const
{
    TArray<FDir_AgentTask> AllTasks;
    
    for (const auto& TaskPair : AgentTasks)
    {
        AllTasks.Add(TaskPair.Value);
    }
    
    // Sort by Agent ID
    AllTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.AgentID < B.AgentID;
    });
    
    return AllTasks;
}

FDir_ProductionMetrics UProductionCoordinator::GetProductionMetrics() const
{
    return ProductionMetrics;
}

void UProductionCoordinator::AdvanceProductionPhase(EDir_ProductionPhase NewPhase, const FString& Milestone)
{
    ProductionMetrics.CurrentPhase = NewPhase;
    ProductionMetrics.CurrentMilestone = Milestone;
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Advanced to phase %d - %s"), 
           (int32)NewPhase, *Milestone);
}

void UProductionCoordinator::SetCurrentMilestone(const FString& Milestone)
{
    ProductionMetrics.CurrentMilestone = Milestone;
    UE_LOG(LogTemp, Log, TEXT("ProductionCoordinator: Set milestone: %s"), *Milestone);
}

bool UProductionCoordinator::IsMilestoneComplete(const FString& Milestone) const
{
    return CompletedMilestones.Contains(Milestone);
}

void UProductionCoordinator::RecordBuildStatus(bool bSuccess, const FString& BuildLog)
{
    bLastBuildSuccessful = bSuccess;
    LastBuildLog = BuildLog;
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Build status - %s"), 
           bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    if (!BuildLog.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Build Log: %s"), *BuildLog);
    }
}

void UProductionCoordinator::UpdateProductionMetrics()
{
    ProductionMetrics.TotalCycles++;
    ProductionMetrics.ActiveAgents = 0;
    ProductionMetrics.BlockedAgents = 0;
    
    float TotalProgress = 0.0f;
    
    for (const auto& TaskPair : AgentTasks)
    {
        const FDir_AgentTask& Task = TaskPair.Value;
        TotalProgress += Task.Progress;
        
        switch (Task.Status)
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
    }
    
    ProductionMetrics.OverallProgress = AgentTasks.Num() > 0 ? TotalProgress / AgentTasks.Num() : 0.0f;
}

void UProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d | Milestone: %s"), 
           (int32)ProductionMetrics.CurrentPhase, *ProductionMetrics.CurrentMilestone);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), ProductionMetrics.OverallProgress * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d | Blocked: %d"), 
           ProductionMetrics.ActiveAgents, ProductionMetrics.BlockedAgents);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d | Total Cycles: %d"), 
           ProductionMetrics.CompletedTasks, ProductionMetrics.TotalCycles);
    
    for (const auto& TaskPair : AgentTasks)
    {
        const FDir_AgentTask& Task = TaskPair.Value;
        UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s - %.1f%% - %s"), 
               Task.AgentID, *Task.AgentName, 
               *UEnum::GetValueAsString(Task.Status), 
               Task.Progress * 100.0f, *Task.CurrentTask);
    }
}

void UProductionCoordinator::GenerateProductionReport()
{
    FString ReportContent = TEXT("=== TRANSPERSONAL GAME STUDIO - PRODUCTION REPORT ===\n\n");
    
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(ProductionMetrics.CurrentPhase));
    ReportContent += FString::Printf(TEXT("Current Milestone: %s\n"), *ProductionMetrics.CurrentMilestone);
    ReportContent += FString::Printf(TEXT("Overall Progress: %.1f%%\n\n"), ProductionMetrics.OverallProgress * 100.0f);
    
    ReportContent += TEXT("AGENT STATUS:\n");
    for (const auto& TaskPair : AgentTasks)
    {
        const FDir_AgentTask& Task = TaskPair.Value;
        ReportContent += FString::Printf(TEXT("#%02d %-25s [%s] %.1f%% - %s\n"), 
                                        Task.AgentID, *Task.AgentName, 
                                        *UEnum::GetValueAsString(Task.Status),
                                        Task.Progress * 100.0f, *Task.CurrentTask);
        
        if (!Task.LastDeliverable.IsEmpty())
        {
            ReportContent += FString::Printf(TEXT("     Last Deliverable: %s\n"), *Task.LastDeliverable);
        }
    }
    
    // Save report to file
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("ProductionReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Report saved to %s"), *ReportPath);
}