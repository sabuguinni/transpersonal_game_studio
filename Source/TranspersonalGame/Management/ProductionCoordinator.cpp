#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentCycle = 1;
    CurrentPhase = TEXT("Initialization");
    OverallProgress = 0.0f;
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the production pipeline
    InitializeProductionPipeline();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Production pipeline initialized"));
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update production metrics every frame
    UpdateProductionMetrics();
}

void AProductionCoordinator::InitializeProductionPipeline()
{
    // Clear existing data
    AgentTasks.Empty();
    ProductionMilestones.Empty();
    
    // Setup initial milestones
    SetupInitialMilestones();
    
    // Assign initial tasks to agents
    AssignInitialTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Pipeline initialized with %d milestones and %d tasks"), 
           ProductionMilestones.Num(), AgentTasks.Num());
}

void AProductionCoordinator::SetupInitialMilestones()
{
    // Milestone 1: Walk Around Prototype
    FDir_ProductionMilestone WalkAroundMilestone;
    WalkAroundMilestone.Type = EDir_MilestoneType::WalkAround;
    WalkAroundMilestone.Description = TEXT("Player can walk around with WASD, camera follows, basic terrain exists");
    WalkAroundMilestone.RequiredAgents.Add(TEXT("Agent_02_EngineArchitect"));
    WalkAroundMilestone.RequiredAgents.Add(TEXT("Agent_03_CoreSystems"));
    WalkAroundMilestone.RequiredAgents.Add(TEXT("Agent_05_WorldGenerator"));
    WalkAroundMilestone.RequiredAgents.Add(TEXT("Agent_09_CharacterArtist"));
    WalkAroundMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(2);
    ProductionMilestones.Add(WalkAroundMilestone);
    
    // Milestone 2: Dinosaur AI Systems
    FDir_ProductionMilestone DinosaurAIMilestone;
    DinosaurAIMilestone.Type = EDir_MilestoneType::DinosaurAI;
    DinosaurAIMilestone.Description = TEXT("5+ dinosaurs with basic AI, collision, and visual feedback");
    DinosaurAIMilestone.RequiredAgents.Add(TEXT("Agent_10_Animation"));
    DinosaurAIMilestone.RequiredAgents.Add(TEXT("Agent_11_NPCBehavior"));
    DinosaurAIMilestone.RequiredAgents.Add(TEXT("Agent_12_CombatAI"));
    DinosaurAIMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(4);
    ProductionMilestones.Add(DinosaurAIMilestone);
    
    // Milestone 3: Survival Systems
    FDir_ProductionMilestone SurvivalMilestone;
    SurvivalMilestone.Type = EDir_MilestoneType::SurvivalSystems;
    SurvivalMilestone.Description = TEXT("Health, hunger, thirst, stamina systems with HUD");
    SurvivalMilestone.RequiredAgents.Add(TEXT("Agent_14_QuestDesigner"));
    SurvivalMilestone.RequiredAgents.Add(TEXT("Agent_16_Audio"));
    SurvivalMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(6);
    ProductionMilestones.Add(SurvivalMilestone);
    
    // Milestone 4: Full Prototype
    FDir_ProductionMilestone FullPrototypeMilestone;
    FullPrototypeMilestone.Type = EDir_MilestoneType::FullPrototype;
    FullPrototypeMilestone.Description = TEXT("Complete playable prototype with all systems integrated");
    FullPrototypeMilestone.RequiredAgents.Add(TEXT("Agent_17_VFX"));
    FullPrototypeMilestone.RequiredAgents.Add(TEXT("Agent_18_QA"));
    FullPrototypeMilestone.RequiredAgents.Add(TEXT("Agent_19_Integration"));
    FullPrototypeMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(10);
    ProductionMilestones.Add(FullPrototypeMilestone);
}

void AProductionCoordinator::AssignInitialTasks()
{
    // High priority tasks for immediate execution
    AssignTaskToAgent(TEXT("Agent_02_EngineArchitect"), TEXT("Validate core engine architecture and compilation"), 10.0f);
    AssignTaskToAgent(TEXT("Agent_03_CoreSystems"), TEXT("Implement physics and collision systems"), 9.0f);
    AssignTaskToAgent(TEXT("Agent_05_WorldGenerator"), TEXT("Create terrain with hills and basic landscape"), 8.0f);
    AssignTaskToAgent(TEXT("Agent_09_CharacterArtist"), TEXT("Refine TranspersonalCharacter with proper movement"), 8.0f);
    AssignTaskToAgent(TEXT("Agent_10_Animation"), TEXT("Add walking, running, jumping animations"), 7.0f);
    
    // Medium priority tasks
    AssignTaskToAgent(TEXT("Agent_12_CombatAI"), TEXT("Implement basic dinosaur AI and behavior trees"), 6.0f);
    AssignTaskToAgent(TEXT("Agent_11_NPCBehavior"), TEXT("Create dinosaur patrol and idle behaviors"), 5.0f);
    AssignTaskToAgent(TEXT("Agent_06_EnvironmentArtist"), TEXT("Add vegetation and environmental props"), 4.0f);
    AssignTaskToAgent(TEXT("Agent_08_LightingAtmosphere"), TEXT("Setup day/night cycle and atmospheric lighting"), 4.0f);
    
    // Lower priority tasks
    AssignTaskToAgent(TEXT("Agent_14_QuestDesigner"), TEXT("Design survival mechanics and objectives"), 3.0f);
    AssignTaskToAgent(TEXT("Agent_16_Audio"), TEXT("Add ambient sounds and character audio"), 3.0f);
    AssignTaskToAgent(TEXT("Agent_17_VFX"), TEXT("Create particle effects for environment"), 2.0f);
    AssignTaskToAgent(TEXT("Agent_18_QA"), TEXT("Setup testing framework and validation"), 2.0f);
    AssignTaskToAgent(TEXT("Agent_19_Integration"), TEXT("Prepare build integration pipeline"), 1.0f);
}

void AProductionCoordinator::AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority)
{
    FDir_AgentTask NewTask;
    NewTask.AgentName = AgentName;
    NewTask.TaskDescription = TaskDescription;
    NewTask.Status = EDir_AgentStatus::Working;
    NewTask.Priority = Priority;
    NewTask.AssignedTime = FDateTime::Now();
    
    AgentTasks.Add(NewTask);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Assigned task to %s: %s (Priority: %.1f)"), 
           *AgentName, *TaskDescription, Priority);
}

void AProductionCoordinator::UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            if (NewStatus == EDir_AgentStatus::Complete)
            {
                Task.CompletionTime = FDateTime::Now();
            }
            break;
        }
    }
}

void AProductionCoordinator::CompleteAgentTask(const FString& AgentName)
{
    UpdateAgentStatus(AgentName, EDir_AgentStatus::Complete);
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Task completed by %s"), *AgentName);
}

bool AProductionCoordinator::ValidateMilestone(EDir_MilestoneType MilestoneType)
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.Type == MilestoneType)
        {
            // Check if all required agents have completed their tasks
            int32 CompletedAgents = 0;
            for (const FString& RequiredAgent : Milestone.RequiredAgents)
            {
                for (const FDir_AgentTask& Task : AgentTasks)
                {
                    if (Task.AgentName == RequiredAgent && Task.Status == EDir_AgentStatus::Complete)
                    {
                        CompletedAgents++;
                        break;
                    }
                }
            }
            
            Milestone.CompletionPercentage = (float)CompletedAgents / (float)Milestone.RequiredAgents.Num() * 100.0f;
            Milestone.bIsComplete = (CompletedAgents == Milestone.RequiredAgents.Num());
            
            return Milestone.bIsComplete;
        }
    }
    return false;
}

FString AProductionCoordinator::GetProductionReport()
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT - CYCLE %d ===\n"), CurrentCycle);
    Report += FString::Printf(TEXT("Current Phase: %s\n"), *CurrentPhase);
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n\n"), OverallProgress);
    
    Report += TEXT("=== MILESTONE STATUS ===\n");
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        FString MilestoneStatus = Milestone.bIsComplete ? TEXT("COMPLETE") : TEXT("IN PROGRESS");
        Report += FString::Printf(TEXT("%s: %s (%.1f%%)\n"), 
                                 *Milestone.Description, *MilestoneStatus, Milestone.CompletionPercentage);
    }
    
    Report += TEXT("\n=== AGENT STATUS ===\n");
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusString;
        switch (Task.Status)
        {
            case EDir_AgentStatus::Idle: StatusString = TEXT("IDLE"); break;
            case EDir_AgentStatus::Working: StatusString = TEXT("WORKING"); break;
            case EDir_AgentStatus::Blocked: StatusString = TEXT("BLOCKED"); break;
            case EDir_AgentStatus::Complete: StatusString = TEXT("COMPLETE"); break;
            case EDir_AgentStatus::Failed: StatusString = TEXT("FAILED"); break;
        }
        Report += FString::Printf(TEXT("%s: %s [%s]\n"), 
                                 *Task.AgentName, *StatusString, *Task.TaskDescription);
    }
    
    return Report;
}

TArray<FString> AProductionCoordinator::GetBlockedAgents()
{
    TArray<FString> BlockedAgents;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked || Task.Status == EDir_AgentStatus::Failed)
        {
            BlockedAgents.Add(Task.AgentName);
        }
    }
    return BlockedAgents;
}

void AProductionCoordinator::AdvanceToNextCycle()
{
    CurrentCycle++;
    
    // Update phase based on milestone completion
    if (ValidateMilestone(EDir_MilestoneType::FullPrototype))
    {
        CurrentPhase = TEXT("Production Complete");
    }
    else if (ValidateMilestone(EDir_MilestoneType::SurvivalSystems))
    {
        CurrentPhase = TEXT("Final Integration");
    }
    else if (ValidateMilestone(EDir_MilestoneType::DinosaurAI))
    {
        CurrentPhase = TEXT("Survival Systems");
    }
    else if (ValidateMilestone(EDir_MilestoneType::WalkAround))
    {
        CurrentPhase = TEXT("Dinosaur AI");
    }
    else
    {
        CurrentPhase = TEXT("Core Prototype");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Advanced to Cycle %d - Phase: %s"), 
           CurrentCycle, *CurrentPhase);
}

void AProductionCoordinator::UpdateProductionMetrics()
{
    // Calculate overall progress based on completed milestones
    float TotalProgress = 0.0f;
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        TotalProgress += Milestone.CompletionPercentage;
    }
    
    if (ProductionMilestones.Num() > 0)
    {
        OverallProgress = TotalProgress / (float)ProductionMilestones.Num();
    }
}

void AProductionCoordinator::GenerateProductionReport()
{
    FString Report = GetProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                                        FString::Printf(TEXT("Production Report Generated - Cycle %d"), CurrentCycle));
    }
}