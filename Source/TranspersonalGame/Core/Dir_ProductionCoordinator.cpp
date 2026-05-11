#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    bProductionActive = true;
    CurrentExecutingAgent = 1;
}

void UDir_ProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized - Studio Director active"));
    
    InitializeAgentNames();
    UpdateProductionMetrics();
    
    // Set up initial tasks for the production pipeline
    FDir_AgentTask InitialTask;
    InitialTask.AgentID = 1;
    InitialTask.AgentName = TEXT("Studio Director");
    InitialTask.TaskDescription = TEXT("Initialize production pipeline and coordinate agents");
    InitialTask.Status = EDir_AgentStatus::Working;
    InitialTask.Priority = 10.0f;
    AddAgentTask(InitialTask);
}

void UDir_ProductionCoordinator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator shutting down"));
    Super::Deinitialize();
}

void UDir_ProductionCoordinator::InitializeAgentNames()
{
    AgentNames.Empty();
    AgentNames.Add(1, TEXT("Studio Director"));
    AgentNames.Add(2, TEXT("Engine Architect"));
    AgentNames.Add(3, TEXT("Core Systems Programmer"));
    AgentNames.Add(4, TEXT("Performance Optimizer"));
    AgentNames.Add(5, TEXT("Procedural World Generator"));
    AgentNames.Add(6, TEXT("Environment Artist"));
    AgentNames.Add(7, TEXT("Architecture & Interior Agent"));
    AgentNames.Add(8, TEXT("Lighting & Atmosphere Agent"));
    AgentNames.Add(9, TEXT("Character Artist Agent"));
    AgentNames.Add(10, TEXT("Animation Agent"));
    AgentNames.Add(11, TEXT("NPC Behavior Agent"));
    AgentNames.Add(12, TEXT("Combat & Enemy AI Agent"));
    AgentNames.Add(13, TEXT("Crowd & Traffic Simulation"));
    AgentNames.Add(14, TEXT("Quest & Mission Designer"));
    AgentNames.Add(15, TEXT("Narrative & Dialogue Agent"));
    AgentNames.Add(16, TEXT("Audio Agent"));
    AgentNames.Add(17, TEXT("VFX Agent"));
    AgentNames.Add(18, TEXT("QA & Testing Agent"));
    AgentNames.Add(19, TEXT("Integration & Build Agent"));
}

void UDir_ProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    if (CurrentPhase != NewPhase)
    {
        CurrentPhase = NewPhase;
        UE_LOG(LogTemp, Warning, TEXT("Production phase changed to: %d"), (int32)NewPhase);
        
        // Update metrics when phase changes
        UpdateProductionMetrics();
    }
}

void UDir_ProductionCoordinator::AddAgentTask(const FDir_AgentTask& Task)
{
    AgentTasks.Add(Task);
    UE_LOG(LogTemp, Log, TEXT("Added task for Agent #%d: %s"), Task.AgentID, *Task.TaskDescription);
}

void UDir_ProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID && Task.Status != EDir_AgentStatus::Completed)
        {
            Task.Status = NewStatus;
            
            if (NewStatus == EDir_AgentStatus::Working)
            {
                Task.StartTime = FDateTime::Now();
            }
            
            UE_LOG(LogTemp, Log, TEXT("Agent #%d status updated to: %d"), AgentID, (int32)NewStatus);
            break;
        }
    }
    
    UpdateProductionMetrics();
}

void UDir_ProductionCoordinator::CompleteAgentTask(int32 AgentID, const TArray<FString>& Deliverables)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID && Task.Status != EDir_AgentStatus::Completed)
        {
            Task.Status = EDir_AgentStatus::Completed;
            Task.CompletionTime = FDateTime::Now();
            Task.Deliverables = Deliverables;
            
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d completed task: %s"), AgentID, *Task.TaskDescription);
            
            // Log deliverables
            for (const FString& Deliverable : Deliverables)
            {
                UE_LOG(LogTemp, Log, TEXT("  Deliverable: %s"), *Deliverable);
            }
            
            break;
        }
    }
    
    UpdateProductionMetrics();
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetTasksForAgent(int32 AgentID) const
{
    TArray<FDir_AgentTask> AgentSpecificTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            AgentSpecificTasks.Add(Task);
        }
    }
    
    return AgentSpecificTasks;
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetBlockedTasks() const
{
    TArray<FDir_AgentTask> BlockedTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedTasks.Add(Task);
        }
    }
    
    return BlockedTasks;
}

bool UDir_ProductionCoordinator::CanAgentProceed(int32 AgentID) const
{
    // Check if all dependencies are completed
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            for (int32 DependencyID : Task.Dependencies)
            {
                bool bDependencyCompleted = false;
                for (const FDir_AgentTask& DepTask : AgentTasks)
                {
                    if (DepTask.AgentID == DependencyID && DepTask.Status == EDir_AgentStatus::Completed)
                    {
                        bDependencyCompleted = true;
                        break;
                    }
                }
                
                if (!bDependencyCompleted)
                {
                    return false;
                }
            }
        }
    }
    
    return true;
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    CurrentMetrics.LastUpdate = FDateTime::Now();
    
    // Count task statuses
    CurrentMetrics.CompletedTasks = 0;
    CurrentMetrics.BlockedTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CurrentMetrics.CompletedTasks++;
        }
        else if (Task.Status == EDir_AgentStatus::Blocked)
        {
            CurrentMetrics.BlockedTasks++;
        }
    }
    
    UpdateMetricsFromWorld();
}

void UDir_ProductionCoordinator::UpdateMetricsFromWorld()
{
    if (UWorld* World = GetWorld())
    {
        // Count actors in world
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentMetrics.TotalActorsInWorld = AllActors.Num();
        
        // Get basic performance metrics
        CurrentMetrics.AverageFrameRate = 1.0f / World->GetDeltaSeconds();
        
        // Memory usage (basic estimation)
        CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    }
}

void UDir_ProductionCoordinator::EmergencyStopProduction()
{
    bProductionActive = false;
    
    // Set all working tasks to blocked
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Blocked;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY STOP: Production pipeline halted"));
}

void UDir_ProductionCoordinator::ResetProductionPipeline()
{
    AgentTasks.Empty();
    CurrentExecutingAgent = 1;
    bProductionActive = true;
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline reset"));
}

bool UDir_ProductionCoordinator::ValidateCurrentBuild()
{
    // Basic build validation
    bool bIsValid = true;
    
    // Check for critical systems
    if (CurrentMetrics.TotalActorsInWorld < 5)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build validation: Too few actors in world"));
        bIsValid = false;
    }
    
    if (CurrentMetrics.BlockedTasks > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build validation: Blocked tasks detected"));
        bIsValid = false;
    }
    
    return bIsValid;
}

TArray<FString> UDir_ProductionCoordinator::GetBuildErrors() const
{
    TArray<FString> Errors;
    
    // Collect errors from blocked tasks
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || Task.Status == EDir_AgentStatus::Failed)
        {
            Errors.Add(FString::Printf(TEXT("Agent #%d blocked/failed: %s"), Task.AgentID, *Task.TaskDescription));
        }
    }
    
    return Errors;
}

int32 UDir_ProductionCoordinator::GetNextAgentToExecute() const
{
    // Return the next agent in sequence that can proceed
    for (int32 AgentID = CurrentExecutingAgent; AgentID <= 19; AgentID++)
    {
        if (CanAgentProceed(AgentID))
        {
            return AgentID;
        }
    }
    
    return -1; // No agent can proceed
}

void UDir_ProductionCoordinator::NotifyAgentStarted(int32 AgentID)
{
    CurrentExecutingAgent = AgentID;
    UpdateAgentStatus(AgentID, EDir_AgentStatus::Working);
    
    UE_LOG(LogTemp, Warning, TEXT("Agent #%d started execution"), AgentID);
}

void UDir_ProductionCoordinator::NotifyAgentCompleted(int32 AgentID)
{
    UpdateAgentStatus(AgentID, EDir_AgentStatus::Completed);
    
    // Move to next agent
    if (AgentID == CurrentExecutingAgent)
    {
        CurrentExecutingAgent = AgentID + 1;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent #%d completed execution"), AgentID);
}