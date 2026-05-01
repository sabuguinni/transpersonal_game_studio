#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize production metrics
    ProductionMetrics.TotalCycles = 0;
    ProductionMetrics.CompletedTasks = 0;
    ProductionMetrics.ActiveAgents = 0;
    ProductionMetrics.OverallProgress = 0.0f;
    ProductionMetrics.CurrentPhase = EDir_ProductionPhase::CoreSystems;
    ProductionMetrics.CurrentMilestone = TEXT("Milestone 1: Walk Around");
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgents();
    bProductionActive = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized - 19 agents ready"));
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bProductionActive)
        return;
    
    LastCycleTime += DeltaTime;
    
    if (LastCycleTime >= CycleUpdateInterval)
    {
        UpdateProductionMetrics();
        LastCycleTime = 0.0f;
    }
}

void UDir_ProductionCoordinator::InitializeAgents()
{
    AgentTasks.Empty();
    
    // Initialize all 19 agents with their roles
    TArray<FString> AgentNames = {
        TEXT("Studio Director"),           // #01
        TEXT("Engine Architect"),         // #02
        TEXT("Core Systems Programmer"),  // #03
        TEXT("Performance Optimizer"),    // #04
        TEXT("Procedural World Generator"), // #05
        TEXT("Environment Artist"),       // #06
        TEXT("Architecture & Interior"),  // #07
        TEXT("Lighting & Atmosphere"),    // #08
        TEXT("Character Artist"),         // #09
        TEXT("Animation Agent"),          // #10
        TEXT("NPC Behavior Agent"),       // #11
        TEXT("Combat & Enemy AI"),        // #12
        TEXT("Crowd & Traffic Simulation"), // #13
        TEXT("Quest & Mission Designer"), // #14
        TEXT("Narrative & Dialogue"),     // #15
        TEXT("Audio Agent"),              // #16
        TEXT("VFX Agent"),               // #17
        TEXT("QA & Testing Agent"),      // #18
        TEXT("Integration & Build Agent") // #19
    };
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.CurrentTask = TEXT("Awaiting coordination");
        NewTask.ProgressPercentage = 0.0f;
        NewTask.LastUpdateTime = FDateTime::Now();
        
        AgentTasks.Add(NewTask);
    }
    
    ProductionMetrics.ActiveAgents = AgentTasks.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agents for production pipeline"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    if (AgentID < 1 || AgentID > AgentTasks.Num())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Agent ID: %d"), AgentID);
        return;
    }
    
    FDir_AgentTask& Task = AgentTasks[AgentID - 1];
    Task.Status = NewStatus;
    Task.CurrentTask = TaskDescription;
    Task.LastUpdateTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s) status updated: %s"), 
           AgentID, *Task.AgentName, *TaskDescription);
}

void UDir_ProductionCoordinator::SetAgentProgress(int32 AgentID, float ProgressPercentage)
{
    if (AgentID < 1 || AgentID > AgentTasks.Num())
        return;
    
    FDir_AgentTask& Task = AgentTasks[AgentID - 1];
    Task.ProgressPercentage = FMath::Clamp(ProgressPercentage, 0.0f, 100.0f);
    Task.LastUpdateTime = FDateTime::Now();
    
    if (Task.ProgressPercentage >= 100.0f)
    {
        Task.Status = EDir_AgentStatus::Completed;
        ProductionMetrics.CompletedTasks++;
    }
}

FDir_AgentTask UDir_ProductionCoordinator::GetAgentTask(int32 AgentID) const
{
    if (AgentID < 1 || AgentID > AgentTasks.Num())
    {
        return FDir_AgentTask();
    }
    
    return AgentTasks[AgentID - 1];
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetAllAgentTasks() const
{
    return AgentTasks;
}

void UDir_ProductionCoordinator::StartProductionCycle()
{
    ProductionMetrics.TotalCycles++;
    
    // Reset agent statuses for new cycle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            Task.Status = EDir_AgentStatus::Idle;
            Task.ProgressPercentage = 0.0f;
        }
    }
    
    // Set Studio Director (Agent #1) to working
    if (AgentTasks.Num() > 0)
    {
        AgentTasks[0].Status = EDir_AgentStatus::Working;
        AgentTasks[0].CurrentTask = TEXT("Coordinating production cycle");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production Cycle #%d started"), ProductionMetrics.TotalCycles);
}

void UDir_ProductionCoordinator::AdvanceToNextPhase()
{
    // Check if current phase is complete
    int32 CompletedAgents = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
            CompletedAgents++;
    }
    
    if (CompletedAgents >= AgentTasks.Num() * 0.8f) // 80% completion threshold
    {
        switch (ProductionMetrics.CurrentPhase)
        {
            case EDir_ProductionPhase::PreProduction:
                ProductionMetrics.CurrentPhase = EDir_ProductionPhase::CoreSystems;
                break;
            case EDir_ProductionPhase::CoreSystems:
                ProductionMetrics.CurrentPhase = EDir_ProductionPhase::WorldBuilding;
                break;
            case EDir_ProductionPhase::WorldBuilding:
                ProductionMetrics.CurrentPhase = EDir_ProductionPhase::CharacterCreation;
                break;
            case EDir_ProductionPhase::CharacterCreation:
                ProductionMetrics.CurrentPhase = EDir_ProductionPhase::GameplayMechanics;
                break;
            case EDir_ProductionPhase::GameplayMechanics:
                ProductionMetrics.CurrentPhase = EDir_ProductionPhase::Polish;
                break;
            case EDir_ProductionPhase::Polish:
                ProductionMetrics.CurrentPhase = EDir_ProductionPhase::Testing;
                break;
            default:
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Advanced to next production phase"));
    }
}

bool UDir_ProductionCoordinator::CanAgentProceed(int32 AgentID) const
{
    if (AgentID < 1 || AgentID > AgentTasks.Num())
        return false;
    
    // Agent #1 (Studio Director) can always proceed
    if (AgentID == 1)
        return true;
    
    // Check if previous agent has completed their task
    if (AgentID > 1)
    {
        const FDir_AgentTask& PreviousAgent = AgentTasks[AgentID - 2];
        return PreviousAgent.Status == EDir_AgentStatus::Completed;
    }
    
    return ValidateAgentDependencies(AgentID);
}

void UDir_ProductionCoordinator::BlockAgent(int32 AgentID, const FString& Reason)
{
    if (AgentID < 1 || AgentID > AgentTasks.Num())
        return;
    
    FDir_AgentTask& Task = AgentTasks[AgentID - 1];
    Task.Status = EDir_AgentStatus::Blocked;
    Task.CurrentTask = FString::Printf(TEXT("BLOCKED: %s"), *Reason);
    Task.LastUpdateTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Error, TEXT("Agent #%d (%s) blocked: %s"), 
           AgentID, *Task.AgentName, *Reason);
}

FDir_ProductionMetrics UDir_ProductionCoordinator::GetProductionMetrics() const
{
    return ProductionMetrics;
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    FString Report = TEXT("=== PRODUCTION REPORT ===\n");
    Report += FString::Printf(TEXT("Cycle: %d\n"), ProductionMetrics.TotalCycles);
    Report += FString::Printf(TEXT("Phase: %s\n"), 
                             *UEnum::GetValueAsString(ProductionMetrics.CurrentPhase));
    Report += FString::Printf(TEXT("Milestone: %s\n"), *ProductionMetrics.CurrentMilestone);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), ProductionMetrics.OverallProgress);
    Report += TEXT("\nAgent Status:\n");
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        Report += FString::Printf(TEXT("#%02d %s: %s (%.1f%%)\n"),
                                 Task.AgentID,
                                 *Task.AgentName,
                                 *UEnum::GetValueAsString(Task.Status),
                                 Task.ProgressPercentage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void UDir_ProductionCoordinator::ResetProduction()
{
    bProductionActive = false;
    
    ProductionMetrics.TotalCycles = 0;
    ProductionMetrics.CompletedTasks = 0;
    ProductionMetrics.OverallProgress = 0.0f;
    ProductionMetrics.CurrentPhase = EDir_ProductionPhase::PreProduction;
    
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.ProgressPercentage = 0.0f;
        Task.CurrentTask = TEXT("Awaiting reset");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline reset"));
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    int32 CompletedCount = 0;
    float TotalProgress = 0.0f;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
            CompletedCount++;
        
        TotalProgress += Task.ProgressPercentage;
    }
    
    ProductionMetrics.CompletedTasks = CompletedCount;
    ProductionMetrics.OverallProgress = AgentTasks.Num() > 0 ? 
                                       TotalProgress / AgentTasks.Num() : 0.0f;
    
    // Update milestone progress
    if (ProductionMetrics.OverallProgress >= 25.0f && 
        ProductionMetrics.CurrentMilestone.Contains(TEXT("Milestone 1")))
    {
        ProductionMetrics.CurrentMilestone = TEXT("Milestone 2: Dinosaur Interaction");
    }
}

bool UDir_ProductionCoordinator::ValidateAgentDependencies(int32 AgentID) const
{
    // Define critical dependencies
    switch (AgentID)
    {
        case 3: // Core Systems needs Engine Architect
            return AgentTasks[1].Status == EDir_AgentStatus::Completed;
        case 5: // World Generator needs Core Systems
            return AgentTasks[2].Status == EDir_AgentStatus::Completed;
        case 9: // Character Artist needs World Generator
            return AgentTasks[4].Status == EDir_AgentStatus::Completed;
        case 15: // Narrative needs Character Artist
            return AgentTasks[8].Status == EDir_AgentStatus::Completed;
        case 18: // QA needs most systems complete
            return ProductionMetrics.OverallProgress >= 70.0f;
        default:
            return true;
    }
}