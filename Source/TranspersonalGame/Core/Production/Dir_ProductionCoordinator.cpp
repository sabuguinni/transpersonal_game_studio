#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentPhase = EDir_ProductionPhase::PreProduction;
    LastAnalysisTime = 0.0f;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProduction();
    AnalyzeLevelState();
    
    UE_LOG(LogTemp, Warning, TEXT("Director Production Coordinator initialized"));
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastAnalysisTime += DeltaTime;
    
    // Update metrics every 5 seconds
    if (LastAnalysisTime >= 5.0f)
    {
        UpdateMetrics();
        CheckDependencies();
        LastAnalysisTime = 0.0f;
    }
}

void UDir_ProductionCoordinator::InitializeProduction()
{
    // Clear existing tasks
    ActiveTasks.Empty();
    ProductionLog.Empty();
    
    // Initialize core agent tasks for Milestone 1
    AssignTaskToAgent(2, TEXT("Implement core physics and collision systems"), 10.0f);
    AssignTaskToAgent(3, TEXT("Create performance optimization framework"), 9.0f);
    AssignTaskToAgent(5, TEXT("Generate procedural terrain with height variation"), 8.0f);
    AssignTaskToAgent(6, TEXT("Populate world with vegetation and props"), 7.0f);
    AssignTaskToAgent(9, TEXT("Create playable character with WASD movement"), 10.0f);
    AssignTaskToAgent(10, TEXT("Implement character animations and IK"), 6.0f);
    AssignTaskToAgent(11, TEXT("Add basic dinosaur AI and behavior"), 8.0f);
    AssignTaskToAgent(12, TEXT("Create survival HUD with health/hunger bars"), 7.0f);
    AssignTaskToAgent(8, TEXT("Setup lighting and atmosphere"), 6.0f);
    
    LogProductionMilestone(TEXT("Production initialized - Milestone 1: Walk Around prototype"));
}

void UDir_ProductionCoordinator::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentID = AgentID;
    NewTask.AgentName = FString::Printf(TEXT("Agent #%d"), AgentID);
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.Priority = Priority;
    NewTask.StartTime = FDateTime::Now();
    NewTask.EstimatedCompletion = FDateTime::Now() + FTimespan::FromHours(2);
    
    ActiveTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%d: %s"), AgentID, *TaskDescription);
}

void UDir_ProductionCoordinator::UpdateTaskStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            if (NewStatus == EDir_AgentStatus::Complete)
            {
                CurrentMetrics.CompletedTasks++;
                CurrentMetrics.PendingTasks = FMath::Max(0, CurrentMetrics.PendingTasks - 1);
            }
            break;
        }
    }
    
    UpdateMetrics();
}

void UDir_ProductionCoordinator::AnalyzeLevelState()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count all actors in the level
    int32 TotalActors = 0;
    int32 DinoCount = 0;
    int32 CharacterCount = 0;
    int32 EnvironmentCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        TotalActors++;
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Classify actors
        if (ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("veloci")) || 
            ActorLabel.Contains(TEXT("tricera")) || ActorLabel.Contains(TEXT("brachi")) ||
            ActorLabel.Contains(TEXT("ankylo")) || ActorLabel.Contains(TEXT("dino")))
        {
            DinoCount++;
        }
        else if (ActorLabel.Contains(TEXT("character")) || ActorLabel.Contains(TEXT("player")))
        {
            CharacterCount++;
        }
        else if (ActorLabel.Contains(TEXT("tree")) || ActorLabel.Contains(TEXT("rock")) || 
                 ActorLabel.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("terrain")))
        {
            EnvironmentCount++;
        }
    }
    
    // Update metrics
    CurrentMetrics.TotalActorsInLevel = TotalActors;
    CurrentMetrics.DinosaurActors = DinoCount;
    CurrentMetrics.CharacterActors = CharacterCount;
    CurrentMetrics.EnvironmentActors = EnvironmentCount;
    CurrentMetrics.LastUpdate = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("Level Analysis: %d total actors, %d dinosaurs, %d characters, %d environment"), 
           TotalActors, DinoCount, CharacterCount, EnvironmentCount);
}

FDir_ProductionMetrics UDir_ProductionCoordinator::GetProductionMetrics()
{
    UpdateMetrics();
    return CurrentMetrics;
}

void UDir_ProductionCoordinator::AdvanceToNextPhase()
{
    if (!CanAdvancePhase())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot advance phase - blocking issues exist"));
        return;
    }
    
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::CoreSystems;
            LogProductionMilestone(TEXT("Advanced to Core Systems phase"));
            break;
        case EDir_ProductionPhase::CoreSystems:
            CurrentPhase = EDir_ProductionPhase::WorldBuilding;
            LogProductionMilestone(TEXT("Advanced to World Building phase"));
            break;
        case EDir_ProductionPhase::WorldBuilding:
            CurrentPhase = EDir_ProductionPhase::CharacterDevelopment;
            LogProductionMilestone(TEXT("Advanced to Character Development phase"));
            break;
        case EDir_ProductionPhase::CharacterDevelopment:
            CurrentPhase = EDir_ProductionPhase::GameplayImplementation;
            LogProductionMilestone(TEXT("Advanced to Gameplay Implementation phase"));
            break;
        case EDir_ProductionPhase::GameplayImplementation:
            CurrentPhase = EDir_ProductionPhase::PolishAndOptimization;
            LogProductionMilestone(TEXT("Advanced to Polish and Optimization phase"));
            break;
        case EDir_ProductionPhase::PolishAndOptimization:
            CurrentPhase = EDir_ProductionPhase::Testing;
            LogProductionMilestone(TEXT("Advanced to Testing phase"));
            break;
        case EDir_ProductionPhase::Testing:
            CurrentPhase = EDir_ProductionPhase::Release;
            LogProductionMilestone(TEXT("Advanced to Release phase"));
            break;
        default:
            break;
    }
}

bool UDir_ProductionCoordinator::CanAdvancePhase()
{
    // Check if minimum requirements for current phase are met
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            return CurrentMetrics.TotalActorsInLevel > 10;
        case EDir_ProductionPhase::CoreSystems:
            return CurrentMetrics.CharacterActors > 0 && CurrentMetrics.EnvironmentActors > 5;
        case EDir_ProductionPhase::WorldBuilding:
            return CurrentMetrics.DinosaurActors > 3 && CurrentMetrics.EnvironmentActors > 20;
        default:
            return CurrentMetrics.CompletedTasks >= CurrentMetrics.PendingTasks * 0.8f;
    }
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetTasksForAgent(int32 AgentID)
{
    TArray<FDir_AgentTask> AgentTasks;
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID)
        {
            AgentTasks.Add(Task);
        }
    }
    
    return AgentTasks;
}

void UDir_ProductionCoordinator::BlockAgent(int32 AgentID, const FString& Reason)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = EDir_AgentStatus::Blocked;
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent #%d blocked: %s"), AgentID, *Reason);
}

void UDir_ProductionCoordinator::UnblockAgent(int32 AgentID)
{
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.AgentID == AgentID && Task.Status == EDir_AgentStatus::Blocked)
        {
            Task.Status = EDir_AgentStatus::Working;
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent #%d unblocked"), AgentID);
}

bool UDir_ProductionCoordinator::ValidateMinimumPlayableState()
{
    // Check for minimum viable prototype requirements
    bool HasCharacter = CurrentMetrics.CharacterActors > 0;
    bool HasTerrain = CurrentMetrics.EnvironmentActors > 5;
    bool HasDinosaurs = CurrentMetrics.DinosaurActors > 3;
    bool HasMinActors = CurrentMetrics.TotalActorsInLevel > 15;
    
    return HasCharacter && HasTerrain && HasDinosaurs && HasMinActors;
}

TArray<FString> UDir_ProductionCoordinator::GetBlockingIssues()
{
    TArray<FString> Issues;
    
    if (CurrentMetrics.CharacterActors == 0)
    {
        Issues.Add(TEXT("No playable character in level"));
    }
    
    if (CurrentMetrics.DinosaurActors < 3)
    {
        Issues.Add(TEXT("Insufficient dinosaur actors for gameplay"));
    }
    
    if (CurrentMetrics.EnvironmentActors < 5)
    {
        Issues.Add(TEXT("Insufficient environment props"));
    }
    
    // Check for blocked agents
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            Issues.Add(FString::Printf(TEXT("Agent #%d is blocked"), Task.AgentID));
        }
    }
    
    return Issues;
}

FString UDir_ProductionCoordinator::GenerateProductionReport()
{
    FString Report = TEXT("=== PRODUCTION REPORT ===\n");
    Report += FString::Printf(TEXT("Current Phase: %s\n"), 
                             *UEnum::GetValueAsString(CurrentPhase));
    Report += FString::Printf(TEXT("Total Actors: %d\n"), CurrentMetrics.TotalActorsInLevel);
    Report += FString::Printf(TEXT("Dinosaurs: %d\n"), CurrentMetrics.DinosaurActors);
    Report += FString::Printf(TEXT("Characters: %d\n"), CurrentMetrics.CharacterActors);
    Report += FString::Printf(TEXT("Environment: %d\n"), CurrentMetrics.EnvironmentActors);
    Report += FString::Printf(TEXT("Completed Tasks: %d\n"), CurrentMetrics.CompletedTasks);
    Report += FString::Printf(TEXT("Pending Tasks: %d\n"), CurrentMetrics.PendingTasks);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n"), CurrentMetrics.OverallProgress);
    
    TArray<FString> BlockingIssues = GetBlockingIssues();
    if (BlockingIssues.Num() > 0)
    {
        Report += TEXT("\n=== BLOCKING ISSUES ===\n");
        for (const FString& Issue : BlockingIssues)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Issue);
        }
    }
    
    return Report;
}

void UDir_ProductionCoordinator::LogProductionMilestone(const FString& MilestoneDescription)
{
    FString LogEntry = FString::Printf(TEXT("[%s] %s"), 
                                      *FDateTime::Now().ToString(), 
                                      *MilestoneDescription);
    ProductionLog.Add(LogEntry);
    
    UE_LOG(LogTemp, Warning, TEXT("MILESTONE: %s"), *MilestoneDescription);
}

void UDir_ProductionCoordinator::UpdateMetrics()
{
    AnalyzeLevelState();
    
    // Calculate overall progress
    int32 TotalTasks = CurrentMetrics.CompletedTasks + CurrentMetrics.PendingTasks;
    if (TotalTasks > 0)
    {
        CurrentMetrics.OverallProgress = (float)CurrentMetrics.CompletedTasks / TotalTasks * 100.0f;
    }
    
    CurrentMetrics.PendingTasks = 0;
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status != EDir_AgentStatus::Complete)
        {
            CurrentMetrics.PendingTasks++;
        }
    }
}

void UDir_ProductionCoordinator::CheckDependencies()
{
    // Check if any agents are waiting on dependencies
    for (FDir_AgentTask& Task : ActiveTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            // Check if blocking dependencies are resolved
            bool CanUnblock = true;
            for (const FString& Dependency : Task.Dependencies)
            {
                // Check if dependency is completed
                bool DependencyMet = false;
                for (const FDir_AgentTask& OtherTask : ActiveTasks)
                {
                    if (OtherTask.TaskDescription.Contains(Dependency) && 
                        OtherTask.Status == EDir_AgentStatus::Complete)
                    {
                        DependencyMet = true;
                        break;
                    }
                }
                
                if (!DependencyMet)
                {
                    CanUnblock = false;
                    break;
                }
            }
            
            if (CanUnblock)
            {
                Task.Status = EDir_AgentStatus::Working;
                UE_LOG(LogTemp, Warning, TEXT("Agent #%d unblocked - dependencies resolved"), Task.AgentID);
            }
        }
    }
}

void UDir_ProductionCoordinator::OptimizeTaskQueue()
{
    // Sort tasks by priority
    ActiveTasks.Sort([](const FDir_AgentTask& A, const FDir_AgentTask& B) {
        return A.Priority > B.Priority;
    });
}