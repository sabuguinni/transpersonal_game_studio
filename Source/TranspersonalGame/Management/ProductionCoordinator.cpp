#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentCycle = 20; // Starting from Cycle 020
    ProductionPhase = TEXT("Milestone 1 - Walk Around");
    OverallProgress = 0.0f;
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMilestones();
    LogProductionStatus();
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update overall progress every frame
    OverallProgress = CalculateOverallProgress();
}

void AProductionCoordinator::InitializeMilestones()
{
    Milestones.Empty();
    
    // Milestone 1: Walk Around
    FDir_MilestoneData Milestone1;
    Milestone1.MilestoneName = TEXT("Milestone 1 - Walk Around");
    Milestone1.RequiredFeatures.Add(TEXT("ThirdPersonCharacter with WASD movement"));
    Milestone1.RequiredFeatures.Add(TEXT("Camera boom + follow camera"));
    Milestone1.RequiredFeatures.Add(TEXT("Landscape with basic terrain"));
    Milestone1.RequiredFeatures.Add(TEXT("Player can walk, run, jump"));
    Milestone1.RequiredFeatures.Add(TEXT("3-5 static dinosaur meshes placed"));
    Milestone1.RequiredFeatures.Add(TEXT("Directional light + sky + fog"));
    Milestones.Add(Milestone1);
    
    // Milestone 2: Basic Interaction
    FDir_MilestoneData Milestone2;
    Milestone2.MilestoneName = TEXT("Milestone 2 - Basic Interaction");
    Milestone2.RequiredFeatures.Add(TEXT("Pick up objects"));
    Milestone2.RequiredFeatures.Add(TEXT("Basic UI elements"));
    Milestone2.RequiredFeatures.Add(TEXT("Simple inventory system"));
    Milestones.Add(Milestone2);
    
    // Milestone 3: Survival Systems
    FDir_MilestoneData Milestone3;
    Milestone3.MilestoneName = TEXT("Milestone 3 - Survival Systems");
    Milestone3.RequiredFeatures.Add(TEXT("Health/Hunger/Thirst bars"));
    Milestone3.RequiredFeatures.Add(TEXT("Resource gathering"));
    Milestone3.RequiredFeatures.Add(TEXT("Basic crafting"));
    Milestones.Add(Milestone3);
    
    // Initialize agent tasks
    AgentTasks.Empty();
    
    // Agent #2 - Engine Architect
    FDir_AgentTask Task2;
    Task2.AgentNumber = 2;
    Task2.AgentName = TEXT("Engine Architect");
    Task2.TaskDescription = TEXT("Validate core architecture and establish technical standards");
    Task2.CurrentPhase = EDir_ProductionPhase::Production;
    Task2.Priority = 10.0f;
    AgentTasks.Add(Task2);
    
    // Agent #3 - Core Systems
    FDir_AgentTask Task3;
    Task3.AgentNumber = 3;
    Task3.AgentName = TEXT("Core Systems Programmer");
    Task3.TaskDescription = TEXT("Implement physics, collision, and movement systems");
    Task3.CurrentPhase = EDir_ProductionPhase::Production;
    Task3.Priority = 9.0f;
    AgentTasks.Add(Task3);
    
    // Agent #5 - World Generator
    FDir_AgentTask Task5;
    Task5.AgentNumber = 5;
    Task5.AgentName = TEXT("Procedural World Generator");
    Task5.TaskDescription = TEXT("Enhance terrain with height variation and biomes");
    Task5.CurrentPhase = EDir_ProductionPhase::Production;
    Task5.Priority = 8.0f;
    AgentTasks.Add(Task5);
    
    // Agent #9 - Character Artist
    FDir_AgentTask Task9;
    Task9.AgentNumber = 9;
    Task9.AgentName = TEXT("Character Artist");
    Task9.TaskDescription = TEXT("Improve character model and create dinosaur actors");
    Task9.CurrentPhase = EDir_ProductionPhase::Production;
    Task9.Priority = 8.0f;
    AgentTasks.Add(Task9);
    
    // Agent #10 - Animation
    FDir_AgentTask Task10;
    Task10.AgentNumber = 10;
    Task10.AgentName = TEXT("Animation Agent");
    Task10.TaskDescription = TEXT("Implement character and dinosaur animation systems");
    Task10.CurrentPhase = EDir_ProductionPhase::Production;
    Task10.Priority = 7.0f;
    AgentTasks.Add(Task10);
    
    // Agent #12 - Combat & Enemy AI
    FDir_AgentTask Task12;
    Task12.AgentNumber = 12;
    Task12.AgentName = TEXT("Combat & Enemy AI");
    Task12.TaskDescription = TEXT("Create survival HUD and basic dinosaur AI");
    Task12.CurrentPhase = EDir_ProductionPhase::Production;
    Task12.Priority = 7.0f;
    AgentTasks.Add(Task12);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d milestones and %d agent tasks"), 
           Milestones.Num(), AgentTasks.Num());
}

void AProductionCoordinator::UpdateMilestoneProgress(const FString& MilestoneName, float NewProgress)
{
    for (FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.CompletionPercentage = FMath::Clamp(NewProgress, 0.0f, 100.0f);
            Milestone.bIsComplete = (Milestone.CompletionPercentage >= 100.0f);
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Updated %s to %.1f%% complete"), 
                   *MilestoneName, Milestone.CompletionPercentage);
            break;
        }
    }
}

bool AProductionCoordinator::IsMilestoneComplete(const FString& MilestoneName) const
{
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.bIsComplete;
        }
    }
    return false;
}

float AProductionCoordinator::GetMilestoneProgress(const FString& MilestoneName) const
{
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.CompletionPercentage;
        }
    }
    return 0.0f;
}

void AProductionCoordinator::AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.TaskDescription = TaskDescription;
            Task.Priority = Priority;
            Task.bIsBlocked = false;
            Task.BlockingReason = TEXT("");
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Assigned task to Agent #%d: %s"), 
                   AgentNumber, *TaskDescription);
            return;
        }
    }
    
    // Create new task if agent not found
    FDir_AgentTask NewTask;
    NewTask.AgentNumber = AgentNumber;
    NewTask.AgentName = FString::Printf(TEXT("Agent #%d"), AgentNumber);
    NewTask.TaskDescription = TaskDescription;
    NewTask.Priority = Priority;
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Created new task for Agent #%d: %s"), 
           AgentNumber, *TaskDescription);
}

void AProductionCoordinator::CompleteAgentTask(int32 AgentNumber)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.CurrentPhase = EDir_ProductionPhase::Complete;
            Task.bIsBlocked = false;
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent #%d completed task: %s"), 
                   AgentNumber, *Task.TaskDescription);
            break;
        }
    }
}

void AProductionCoordinator::BlockAgent(int32 AgentNumber, const FString& Reason)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.bIsBlocked = true;
            Task.BlockingReason = Reason;
            
            UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Agent #%d blocked - %s"), 
                   AgentNumber, *Reason);
            break;
        }
    }
}

void AProductionCoordinator::UnblockAgent(int32 AgentNumber)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            Task.bIsBlocked = false;
            Task.BlockingReason = TEXT("");
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent #%d unblocked"), AgentNumber);
            break;
        }
    }
}

TArray<FDir_AgentTask> AProductionCoordinator::GetBlockedAgents() const
{
    TArray<FDir_AgentTask> BlockedAgents;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.bIsBlocked)
        {
            BlockedAgents.Add(Task);
        }
    }
    
    return BlockedAgents;
}

void AProductionCoordinator::IncrementCycle()
{
    CurrentCycle++;
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Advanced to Cycle %d"), CurrentCycle);
}

float AProductionCoordinator::CalculateOverallProgress()
{
    if (Milestones.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        TotalProgress += Milestone.CompletionPercentage;
    }
    
    return TotalProgress / Milestones.Num();
}

void AProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS - CYCLE %d ==="), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *ProductionPhase);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), OverallProgress);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestones:"));
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %.1f%% %s"), 
               *Milestone.MilestoneName, 
               Milestone.CompletionPercentage,
               Milestone.bIsComplete ? TEXT("[COMPLETE]") : TEXT("[IN PROGRESS]"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Tasks:"));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString Status = Task.bIsBlocked ? FString::Printf(TEXT("[BLOCKED: %s]"), *Task.BlockingReason) : TEXT("[ACTIVE]");
        UE_LOG(LogTemp, Warning, TEXT("  Agent #%d (%s): %s %s"), 
               Task.AgentNumber, *Task.AgentName, *Task.TaskDescription, *Status);
    }
}

void AProductionCoordinator::RefreshProductionData()
{
    OverallProgress = CalculateOverallProgress();
    LogProductionStatus();
}

void AProductionCoordinator::ResetAllTasks()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.CurrentPhase = EDir_ProductionPhase::Planning;
        Task.bIsBlocked = false;
        Task.BlockingReason = TEXT("");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: All agent tasks reset"));
}

void AProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - CYCLE %d ==="), CurrentCycle);
    
    // Count completed vs incomplete milestones
    int32 CompletedMilestones = 0;
    for (const FDir_MilestoneData& Milestone : Milestones)
    {
        if (Milestone.bIsComplete)
        {
            CompletedMilestones++;
        }
    }
    
    // Count blocked agents
    int32 BlockedAgents = GetBlockedAgents().Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Summary:"));
    UE_LOG(LogTemp, Warning, TEXT("  Milestones Complete: %d/%d"), CompletedMilestones, Milestones.Num());
    UE_LOG(LogTemp, Warning, TEXT("  Overall Progress: %.1f%%"), OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("  Blocked Agents: %d"), BlockedAgents);
    UE_LOG(LogTemp, Warning, TEXT("  Current Phase: %s"), *ProductionPhase);
    
    // Recommendations
    if (BlockedAgents > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: %d agents are blocked and need attention"), BlockedAgents);
    }
    
    if (OverallProgress < 25.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("RECOMMENDATION: Focus on Milestone 1 completion"));
    }
    else if (OverallProgress < 50.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("RECOMMENDATION: Begin Milestone 2 preparation"));
    }
}