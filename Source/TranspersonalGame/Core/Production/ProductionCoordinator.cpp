#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    CurrentPhase = EDir_ProductionPhase::Prototype;
    CurrentCycle = 0;
    ProductionBudget = 100.0f;
    BudgetUsed = 0.0f;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultMilestones();
    StartNewCycle();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: System initialized for Cycle %d"), CurrentCycle);
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateTaskPriorities();
    CheckMilestoneCompletion();
}

void UDir_ProductionCoordinator::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.StartTime = FDateTime::Now();
    
    // Set agent name based on ID
    switch(AgentID)
    {
        case 1: NewTask.AgentName = TEXT("Studio Director"); break;
        case 2: NewTask.AgentName = TEXT("Engine Architect"); break;
        case 3: NewTask.AgentName = TEXT("Core Systems"); break;
        case 4: NewTask.AgentName = TEXT("Performance Optimizer"); break;
        case 5: NewTask.AgentName = TEXT("World Generator"); break;
        case 6: NewTask.AgentName = TEXT("Environment Artist"); break;
        case 7: NewTask.AgentName = TEXT("Architecture Agent"); break;
        case 8: NewTask.AgentName = TEXT("Lighting Agent"); break;
        case 9: NewTask.AgentName = TEXT("Character Artist"); break;
        case 10: NewTask.AgentName = TEXT("Animation Agent"); break;
        case 11: NewTask.AgentName = TEXT("NPC Behavior"); break;
        case 12: NewTask.AgentName = TEXT("Combat AI"); break;
        case 13: NewTask.AgentName = TEXT("Crowd Simulation"); break;
        case 14: NewTask.AgentName = TEXT("Quest Designer"); break;
        case 15: NewTask.AgentName = TEXT("Narrative Agent"); break;
        case 16: NewTask.AgentName = TEXT("Audio Agent"); break;
        case 17: NewTask.AgentName = TEXT("VFX Agent"); break;
        case 18: NewTask.AgentName = TEXT("QA Testing"); break;
        case 19: NewTask.AgentName = TEXT("Integration Agent"); break;
        default: NewTask.AgentName = FString::Printf(TEXT("Agent_%d"), AgentID); break;
    }
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Assigned task to %s (ID:%d) - %s"), 
           *NewTask.AgentName, AgentID, *TaskDescription);
}

void UDir_ProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    for(FDir_AgentTask& Task : ActiveTasks)
    {
        if(Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: %s status updated to %d"), 
                   *Task.AgentName, (int32)NewStatus);
            break;
        }
    }
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetTasksForAgent(int32 AgentID) const
{
    TArray<FDir_AgentTask> AgentTasks;
    
    for(const FDir_AgentTask& Task : ActiveTasks)
    {
        if(Task.AgentID == AgentID)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

bool UDir_ProductionCoordinator::AreAgentDependenciesMet(int32 AgentID) const
{
    for(const FDir_AgentTask& Task : ActiveTasks)
    {
        if(Task.AgentID == AgentID)
        {
            for(int32 DepID : Task.Dependencies)
            {
                bool bDependencyMet = false;
                for(const FDir_AgentTask& DepTask : ActiveTasks)
                {
                    if(DepTask.AgentID == DepID && DepTask.Status == EDir_AgentStatus::Complete)
                    {
                        bDependencyMet = true;
                        break;
                    }
                }
                
                if(!bDependencyMet)
                {
                    return false;
                }
            }
        }
    }
    
    return true;
}

void UDir_ProductionCoordinator::CreateMilestone(const FString& Name, const FString& Description, const TArray<int32>& RequiredAgents)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.MilestoneName = Name;
    NewMilestone.Description = Description;
    NewMilestone.RequiredAgents = RequiredAgents;
    NewMilestone.bIsComplete = false;
    NewMilestone.CompletionPercentage = 0.0f;
    NewMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(3);
    
    Milestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Created milestone '%s'"), *Name);
}

void UDir_ProductionCoordinator::UpdateMilestoneProgress(const FString& MilestoneName, float Progress)
{
    for(FDir_ProductionMilestone& Milestone : Milestones)
    {
        if(Milestone.MilestoneName == MilestoneName)
        {
            Milestone.CompletionPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            Milestone.bIsComplete = (Milestone.CompletionPercentage >= 100.0f);
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Milestone '%s' progress: %.1f%%"), 
                   *MilestoneName, Milestone.CompletionPercentage);
            break;
        }
    }
}

bool UDir_ProductionCoordinator::IsMilestoneComplete(const FString& MilestoneName) const
{
    for(const FDir_ProductionMilestone& Milestone : Milestones)
    {
        if(Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.bIsComplete;
        }
    }
    
    return false;
}

float UDir_ProductionCoordinator::GetOverallProgress() const
{
    if(Milestones.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for(const FDir_ProductionMilestone& Milestone : Milestones)
    {
        TotalProgress += Milestone.CompletionPercentage;
    }
    
    return TotalProgress / Milestones.Num();
}

TArray<int32> UDir_ProductionCoordinator::GetBlockedAgents() const
{
    TArray<int32> BlockedAgents;
    
    for(const FDir_AgentTask& Task : ActiveTasks)
    {
        if(Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.AddUnique(Task.AgentID);
        }
    }
    
    return BlockedAgents;
}

FString UDir_ProductionCoordinator::GetProductionReport() const
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT - CYCLE %d ===\n"), CurrentCycle);
    Report += FString::Printf(TEXT("Phase: %d\n"), (int32)CurrentPhase);
    Report += FString::Printf(TEXT("Budget: %.2f/%.2f (%.1f%% used)\n"), BudgetUsed, ProductionBudget, (BudgetUsed/ProductionBudget)*100.0f);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n\n"), GetOverallProgress());
    
    Report += TEXT("ACTIVE TASKS:\n");
    for(const FDir_AgentTask& Task : ActiveTasks)
    {
        FString StatusText;
        switch(Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusText = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusText = TEXT("WORKING"); break;
            case EDir_AgentStatus::Blocked: StatusText = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Complete: StatusText = TEXT("COMPLETE"); break;
            case EDir_AgentStatus::Failed: StatusText = TEXT("FAILED"); break;
        }
        
        Report += FString::Printf(TEXT("- %s: %s [%s]\n"), 
                                 *Task.AgentName, *Task.TaskDescription, *StatusText);
    }
    
    Report += TEXT("\nMILESTONES:\n");
    for(const FDir_ProductionMilestone& Milestone : Milestones)
    {
        Report += FString::Printf(TEXT("- %s: %.1f%% %s\n"), 
                                 *Milestone.MilestoneName, 
                                 Milestone.CompletionPercentage,
                                 Milestone.bIsComplete ? TEXT("[COMPLETE]") : TEXT("[IN PROGRESS]"));
    }
    
    return Report;
}

void UDir_ProductionCoordinator::StartNewCycle()
{
    CurrentCycle++;
    
    // Clear completed tasks from previous cycle
    ActiveTasks.RemoveAll([](const FDir_AgentTask& Task) {
        return Task.Status == EDir_AgentStatus::Complete;
    });
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Started Cycle %d"), CurrentCycle);
}

void UDir_ProductionCoordinator::CompleteCycle()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Completed Cycle %d"), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *GetProductionReport());
}

void UDir_ProductionCoordinator::InitializeDefaultMilestones()
{
    // Milestone 1: Walk Around Prototype
    CreateMilestone(TEXT("Walk Around"), TEXT("Player can walk, run, jump in a basic world with dinosaurs"), {3, 5, 9, 10});
    
    // Milestone 2: Survival Systems
    CreateMilestone(TEXT("Survival Core"), TEXT("Health, hunger, thirst, stamina systems functional"), {3, 9, 12});
    
    // Milestone 3: Dinosaur AI
    CreateMilestone(TEXT("Dinosaur Behavior"), TEXT("Basic dinosaur AI with territorial behavior"), {11, 12, 17});
    
    // Milestone 4: World Population
    CreateMilestone(TEXT("Living World"), TEXT("Populated world with vegetation, props, and atmosphere"), {5, 6, 7, 8});
}

void UDir_ProductionCoordinator::UpdateTaskPriorities()
{
    // Increase priority of tasks that are blocking others
    for(FDir_AgentTask& Task : ActiveTasks)
    {
        if(Task.Status == EDir_AgentStatus::Working)
        {
            // Check if other agents are waiting for this one
            int32 BlockedCount = 0;
            for(const FDir_AgentTask& OtherTask : ActiveTasks)
            {
                if(OtherTask.Dependencies.Contains(Task.AgentID) && OtherTask.Status == EDir_AgentStatus::Blocked)
                {
                    BlockedCount++;
                }
            }
            
            if(BlockedCount > 0)
            {
                Task.Priority = FMath::Min(Task.Priority + (BlockedCount * 0.5f), 10.0f);
            }
        }
    }
}

void UDir_ProductionCoordinator::CheckMilestoneCompletion()
{
    for(FDir_ProductionMilestone& Milestone : Milestones)
    {
        if(!Milestone.bIsComplete && Milestone.RequiredAgents.Num() > 0)
        {
            int32 CompletedAgents = 0;
            
            for(int32 AgentID : Milestone.RequiredAgents)
            {
                bool bAgentComplete = false;
                for(const FDir_AgentTask& Task : ActiveTasks)
                {
                    if(Task.AgentID == AgentID && Task.Status == EDir_AgentStatus::Complete)
                    {
                        bAgentComplete = true;
                        break;
                    }
                }
                
                if(bAgentComplete)
                {
                    CompletedAgents++;
                }
            }
            
            float NewProgress = (float)CompletedAgents / (float)Milestone.RequiredAgents.Num() * 100.0f;
            if(NewProgress != Milestone.CompletionPercentage)
            {
                UpdateMilestoneProgress(Milestone.MilestoneName, NewProgress);
            }
        }
    }
}

bool UDir_ProductionCoordinator::ValidateAgentChain() const
{
    // Validate that agent dependencies form a valid chain
    TArray<int32> ProcessedAgents;
    TArray<int32> PendingAgents = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    
    while(PendingAgents.Num() > 0)
    {
        bool bProgressMade = false;
        
        for(int32 i = PendingAgents.Num() - 1; i >= 0; i--)
        {
            int32 AgentID = PendingAgents[i];
            bool bCanProcess = true;
            
            // Check if all dependencies are processed
            for(const FDir_AgentTask& Task : ActiveTasks)
            {
                if(Task.AgentID == AgentID)
                {
                    for(int32 DepID : Task.Dependencies)
                    {
                        if(!ProcessedAgents.Contains(DepID))
                        {
                            bCanProcess = false;
                            break;
                        }
                    }
                    break;
                }
            }
            
            if(bCanProcess)
            {
                ProcessedAgents.Add(AgentID);
                PendingAgents.RemoveAt(i);
                bProgressMade = true;
            }
        }
        
        if(!bProgressMade)
        {
            UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Circular dependency detected in agent chain"));
            return false;
        }
    }
    
    return true;
}