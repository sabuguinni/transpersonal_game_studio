#include "ProductionMetrics.h"
#include "Engine/World.h"
#include "TimerManager.h"

UProductionMetrics::UProductionMetrics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    InitializeAgentRegistry();
}

void UProductionMetrics::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionMetrics: System initialized for cycle %s"), *ProductionStats.CurrentCycle);
    
    // Set initial production phase
    ProductionStats.CurrentPhase = EDir_ProductionPhase::Prototyping;
    ProductionStats.OverallProgress = 0.0f;
    
    UpdateProductionStats();
}

void UProductionMetrics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateProductionStats();
}

void UProductionMetrics::InitializeAgentRegistry()
{
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

    for (const FString& AgentName : AgentNames)
    {
        FDir_AgentMetrics NewMetrics;
        NewMetrics.AgentName = AgentName;
        NewMetrics.Status = EDir_AgentStatus::Idle;
        NewMetrics.TasksCompleted = 0;
        NewMetrics.FilesCreated = 0;
        NewMetrics.CycleTime = 0.0f;
        NewMetrics.SuccessRate = 1.0f;
        
        AgentRegistry.Add(AgentName, NewMetrics);
    }

    ProductionStats.TotalAgents = AgentNames.Num();
}

void UProductionMetrics::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    if (FDir_AgentMetrics* AgentMetrics = AgentRegistry.Find(AgentName))
    {
        AgentMetrics->Status = NewStatus;
        UE_LOG(LogTemp, Log, TEXT("ProductionMetrics: Agent %s status updated to %d"), *AgentName, (int32)NewStatus);
    }
}

void UProductionMetrics::IncrementTasksCompleted(const FString& AgentName)
{
    if (FDir_AgentMetrics* AgentMetrics = AgentRegistry.Find(AgentName))
    {
        AgentMetrics->TasksCompleted++;
        ProductionStats.CompletedTasks++;
        
        UE_LOG(LogTemp, Log, TEXT("ProductionMetrics: Agent %s completed task #%d"), *AgentName, AgentMetrics->TasksCompleted);
    }
}

void UProductionMetrics::RecordFileCreated(const FString& AgentName)
{
    if (FDir_AgentMetrics* AgentMetrics = AgentRegistry.Find(AgentName))
    {
        AgentMetrics->FilesCreated++;
        UE_LOG(LogTemp, Log, TEXT("ProductionMetrics: Agent %s created file #%d"), *AgentName, AgentMetrics->FilesCreated);
    }
}

FDir_ProductionStats UProductionMetrics::GetProductionStats() const
{
    return ProductionStats;
}

TArray<FDir_AgentMetrics> UProductionMetrics::GetAgentMetrics() const
{
    TArray<FDir_AgentMetrics> MetricsArray;
    for (const auto& Pair : AgentRegistry)
    {
        MetricsArray.Add(Pair.Value);
    }
    return MetricsArray;
}

float UProductionMetrics::CalculateOverallProgress() const
{
    if (ProductionStats.TotalAgents == 0)
    {
        return 0.0f;
    }

    int32 TotalPossibleTasks = ProductionStats.TotalAgents * 10; // Assume 10 tasks per agent per cycle
    float Progress = (float)ProductionStats.CompletedTasks / (float)TotalPossibleTasks;
    
    return FMath::Clamp(Progress, 0.0f, 1.0f);
}

void UProductionMetrics::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    ProductionStats.CurrentPhase = NewPhase;
    UE_LOG(LogTemp, Warning, TEXT("ProductionMetrics: Production phase changed to %d"), (int32)NewPhase);
}

void UProductionMetrics::LogProductionState() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s"), *ProductionStats.CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)ProductionStats.CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Progress: %.2f%%"), ProductionStats.OverallProgress * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d/%d"), ProductionStats.ActiveAgents, ProductionStats.TotalAgents);
    UE_LOG(LogTemp, Warning, TEXT("Completed Tasks: %d"), ProductionStats.CompletedTasks);

    for (const auto& Pair : AgentRegistry)
    {
        const FDir_AgentMetrics& Metrics = Pair.Value;
        UE_LOG(LogTemp, Log, TEXT("Agent %s: Status=%d, Tasks=%d, Files=%d, Success=%.2f%%"), 
            *Metrics.AgentName, 
            (int32)Metrics.Status, 
            Metrics.TasksCompleted, 
            Metrics.FilesCreated, 
            Metrics.SuccessRate * 100.0f);
    }
}

void UProductionMetrics::UpdateProductionStats()
{
    int32 ActiveCount = 0;
    int32 TotalTasks = 0;
    
    for (const auto& Pair : AgentRegistry)
    {
        const FDir_AgentMetrics& Metrics = Pair.Value;
        if (Metrics.Status == EDir_AgentStatus::Working || Metrics.Status == EDir_AgentStatus::Complete)
        {
            ActiveCount++;
        }
        TotalTasks += Metrics.TasksCompleted;
    }

    ProductionStats.ActiveAgents = ActiveCount;
    ProductionStats.CompletedTasks = TotalTasks;
    ProductionStats.OverallProgress = CalculateOverallProgress();

    // Update agent metrics array for Blueprint access
    ProductionStats.AgentMetrics.Empty();
    for (const auto& Pair : AgentRegistry)
    {
        ProductionStats.AgentMetrics.Add(Pair.Value);
    }
}

float UProductionMetrics::CalculateAgentSuccessRate(const FDir_AgentMetrics& Metrics) const
{
    if (Metrics.TasksCompleted == 0)
    {
        return 1.0f; // No tasks attempted yet
    }

    // Simple success rate based on files created vs tasks completed
    float FileToTaskRatio = (float)Metrics.FilesCreated / (float)Metrics.TasksCompleted;
    return FMath::Clamp(FileToTaskRatio, 0.0f, 1.0f);
}