#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    InitializeAgents();
    ProductionMetrics.BudgetLimit = 100.0f;
    ProductionMetrics.TotalCycles = 0;
}

void UDir_ProductionCoordinator::SetProductionPhase(EDir_ProductionPhase NewPhase)
{
    if (CurrentPhase != NewPhase)
    {
        UE_LOG(LogTemp, Warning, TEXT("Production Phase changed from %d to %d"), 
               (int32)CurrentPhase, (int32)NewPhase);
        
        CurrentPhase = NewPhase;
        
        // Reset tasks when phase changes
        for (FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.Status == EDir_AgentStatus::Complete)
            {
                Task.Status = EDir_AgentStatus::Idle;
            }
        }
    }
}

void UDir_ProductionCoordinator::AssignTask(int32 AgentID, const FString& AgentName, const FString& TaskDescription, float Priority)
{
    // Find existing task or create new one
    FDir_AgentTask* ExistingTask = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& Task)
    {
        return Task.AgentID == AgentID;
    });

    if (ExistingTask)
    {
        ExistingTask->TaskDescription = TaskDescription;
        ExistingTask->Priority = Priority;
        ExistingTask->Status = EDir_AgentStatus::Working;
        ExistingTask->AssignedTime = FDateTime::Now();
    }
    else
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = AgentID;
        NewTask.AgentName = AgentName;
        NewTask.TaskDescription = TaskDescription;
        NewTask.Priority = Priority;
        NewTask.Status = EDir_AgentStatus::Working;
        NewTask.AssignedTime = FDateTime::Now();
        
        AgentTasks.Add(NewTask);
    }

    UE_LOG(LogTemp, Log, TEXT("Task assigned to Agent #%d (%s): %s"), 
           AgentID, *AgentName, *TaskDescription);
}

void UDir_ProductionCoordinator::CompleteTask(int32 AgentID, bool bSuccess)
{
    FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& T)
    {
        return T.AgentID == AgentID;
    });

    if (Task)
    {
        Task->Status = bSuccess ? EDir_AgentStatus::Complete : EDir_AgentStatus::Failed;
        Task->CompletionTime = FDateTime::Now();
        
        if (bSuccess)
        {
            ProductionMetrics.CompletedTasks++;
        }
        else
        {
            ProductionMetrics.FailedTasks++;
            AddCriticalIssue(FString::Printf(TEXT("Agent #%d failed task: %s"), 
                           AgentID, *Task->TaskDescription));
        }

        UE_LOG(LogTemp, Warning, TEXT("Agent #%d task %s: %s"), 
               AgentID, bSuccess ? TEXT("COMPLETED") : TEXT("FAILED"), *Task->TaskDescription);
    }
}

void UDir_ProductionCoordinator::BlockTask(int32 AgentID, const FString& Reason)
{
    FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& T)
    {
        return T.AgentID == AgentID;
    });

    if (Task)
    {
        Task->Status = EDir_AgentStatus::Blocked;
        AddCriticalIssue(FString::Printf(TEXT("Agent #%d blocked: %s"), AgentID, *Reason));
        
        UE_LOG(LogTemp, Error, TEXT("Agent #%d BLOCKED: %s"), AgentID, *Reason);
    }
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetActiveTasks() const
{
    TArray<FDir_AgentTask> ActiveTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Working || Task.Status == EDir_AgentStatus::Blocked)
        {
            ActiveTasks.Add(Task);
        }
    }
    
    return ActiveTasks;
}

FDir_AgentTask UDir_ProductionCoordinator::GetTaskByAgent(int32 AgentID) const
{
    const FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& T)
    {
        return T.AgentID == AgentID;
    });

    return Task ? *Task : FDir_AgentTask();
}

void UDir_ProductionCoordinator::UpdateMetrics()
{
    ProductionMetrics.TotalCycles++;
    CalculateTaskMetrics();
    
    // Check budget usage (placeholder - would integrate with actual cost tracking)
    ProductionMetrics.BudgetUsed = FMath::Min(ProductionMetrics.BudgetUsed + 5.0f, ProductionMetrics.BudgetLimit);
    
    UE_LOG(LogTemp, Log, TEXT("Production Metrics Updated - Cycle: %d, Budget: %.2f/%.2f"), 
           ProductionMetrics.TotalCycles, ProductionMetrics.BudgetUsed, ProductionMetrics.BudgetLimit);
}

void UDir_ProductionCoordinator::AddCriticalIssue(const FString& Issue)
{
    ProductionMetrics.CriticalIssues.Add(FString::Printf(TEXT("[%s] %s"), 
                                       *FDateTime::Now().ToString(), *Issue));
    
    // Keep only last 10 issues
    if (ProductionMetrics.CriticalIssues.Num() > 10)
    {
        ProductionMetrics.CriticalIssues.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Error, TEXT("CRITICAL ISSUE: %s"), *Issue);
}

void UDir_ProductionCoordinator::CheckMilestone1Progress()
{
    // Milestone 1: "Walk Around" - minimum viable playable prototype
    float Progress = 0.0f;
    
    // Check for character system (20%)
    if (AgentTasks.ContainsByPredicate([](const FDir_AgentTask& T)
        { return T.AgentID == 9 && T.Status == EDir_AgentStatus::Complete; }))
    {
        Progress += 20.0f;
    }
    
    // Check for terrain system (20%)
    if (AgentTasks.ContainsByPredicate([](const FDir_AgentTask& T)
        { return T.AgentID == 5 && T.Status == EDir_AgentStatus::Complete; }))
    {
        Progress += 20.0f;
    }
    
    // Check for lighting (15%)
    if (AgentTasks.ContainsByPredicate([](const FDir_AgentTask& T)
        { return T.AgentID == 8 && T.Status == EDir_AgentStatus::Complete; }))
    {
        Progress += 15.0f;
    }
    
    // Check for animation (15%)
    if (AgentTasks.ContainsByPredicate([](const FDir_AgentTask& T)
        { return T.AgentID == 10 && T.Status == EDir_AgentStatus::Complete; }))
    {
        Progress += 15.0f;
    }
    
    // Check for dinosaur placement (15%)
    if (AgentTasks.ContainsByPredicate([](const FDir_AgentTask& T)
        { return T.AgentID == 12 && T.Status == EDir_AgentStatus::Complete; }))
    {
        Progress += 15.0f;
    }
    
    // Check for core systems (15%)
    if (AgentTasks.ContainsByPredicate([](const FDir_AgentTask& T)
        { return T.AgentID == 3 && T.Status == EDir_AgentStatus::Complete; }))
    {
        Progress += 15.0f;
    }
    
    Milestone1Progress = FMath::Clamp(Progress, 0.0f, 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 Progress: %.1f%%"), Milestone1Progress);
}

void UDir_ProductionCoordinator::LogProductionState()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Active Tasks: %d"), GetActiveTasks().Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d, Failed: %d"), 
           ProductionMetrics.CompletedTasks, ProductionMetrics.FailedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Budget: %.2f/%.2f"), 
           ProductionMetrics.BudgetUsed, ProductionMetrics.BudgetLimit);
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1: %.1f%%"), Milestone1Progress);
    
    for (const FString& Issue : ProductionMetrics.CriticalIssues)
    {
        UE_LOG(LogTemp, Error, TEXT("ISSUE: %s"), *Issue);
    }
}

void UDir_ProductionCoordinator::ResetProduction()
{
    AgentTasks.Empty();
    ProductionMetrics = FDir_ProductionMetrics();
    ProductionMetrics.BudgetLimit = 100.0f;
    Milestone1Progress = 0.0f;
    CurrentPhase = EDir_ProductionPhase::MinimumViablePrototype;
    
    InitializeAgents();
    
    UE_LOG(LogTemp, Warning, TEXT("Production system reset"));
}

void UDir_ProductionCoordinator::InitializeAgents()
{
    // Initialize all 19 agents with default idle tasks
    TArray<FString> AgentNames = {
        TEXT("Studio Director"), TEXT("Engine Architect"), TEXT("Core Systems Programmer"),
        TEXT("Performance Optimizer"), TEXT("Procedural World Generator"), TEXT("Environment Artist"),
        TEXT("Architecture & Interior Agent"), TEXT("Lighting & Atmosphere Agent"), TEXT("Character Artist Agent"),
        TEXT("Animation Agent"), TEXT("NPC Behavior Agent"), TEXT("Combat & Enemy AI Agent"),
        TEXT("Crowd & Traffic Simulation"), TEXT("Quest & Mission Designer"), TEXT("Narrative & Dialogue Agent"),
        TEXT("Audio Agent"), TEXT("VFX Agent"), TEXT("QA & Testing Agent"), TEXT("Integration & Build Agent")
    };
    
    for (int32 i = 0; i < AgentNames.Num(); ++i)
    {
        FDir_AgentTask InitTask;
        InitTask.AgentID = i + 1;
        InitTask.AgentName = AgentNames[i];
        InitTask.TaskDescription = TEXT("Awaiting assignment");
        InitTask.Status = EDir_AgentStatus::Idle;
        InitTask.Priority = 1.0f;
        
        AgentTasks.Add(InitTask);
    }
}

void UDir_ProductionCoordinator::CalculateTaskMetrics()
{
    if (AgentTasks.Num() == 0) return;
    
    float TotalTime = 0.0f;
    int32 CompletedCount = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Complete)
        {
            FTimespan Duration = Task.CompletionTime - Task.AssignedTime;
            TotalTime += Duration.GetTotalMinutes();
            CompletedCount++;
        }
    }
    
    ProductionMetrics.AverageTaskTime = CompletedCount > 0 ? TotalTime / CompletedCount : 0.0f;
}

bool UDir_ProductionCoordinator::ValidateTaskDependencies(int32 AgentID) const
{
    const FDir_AgentTask* Task = AgentTasks.FindByPredicate([AgentID](const FDir_AgentTask& T)
    {
        return T.AgentID == AgentID;
    });

    if (!Task) return false;
    
    for (int32 DepID : Task->Dependencies)
    {
        const FDir_AgentTask* DepTask = AgentTasks.FindByPredicate([DepID](const FDir_AgentTask& T)
        {
            return T.AgentID == DepID;
        });
        
        if (!DepTask || DepTask->Status != EDir_AgentStatus::Complete)
        {
            return false;
        }
    }
    
    return true;
}