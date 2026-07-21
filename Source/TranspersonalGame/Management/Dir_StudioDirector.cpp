#include "Dir_StudioDirector.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

ADir_StudioDirector::ADir_StudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize production state
    CurrentPhase = EDir_ProductionPhase::PrototypePhase;
    OverallProgress = 0.0f;
    ActiveAgentCount = 0;
    CurrentFocus = TEXT("Minimum Viable Playable Prototype");
}

void ADir_StudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    SetupInitialMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized - Production pipeline active"));
}

void ADir_StudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateProductionMetrics();
}

void ADir_StudioDirector::InitializeProductionPipeline()
{
    // Clear existing tasks
    AgentTasks.Empty();
    
    // Initialize all 19 agents with their core responsibilities
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
        TEXT("VFX Agent"),                // #17
        TEXT("QA & Testing Agent"),       // #18
        TEXT("Integration & Build Agent") // #19
    };

    // Create initial tasks for prototype phase
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.ProgressPercentage = 0.0f;
        NewTask.Priority = TEXT("Normal");
        
        // Assign prototype-specific tasks
        switch (i + 1)
        {
            case 2: // Engine Architect
                NewTask.CurrentTask = TEXT("Define core survival mechanics architecture");
                NewTask.Priority = TEXT("Critical");
                break;
            case 3: // Core Systems
                NewTask.CurrentTask = TEXT("Implement character movement and physics");
                NewTask.Priority = TEXT("High");
                break;
            case 5: // World Generator
                NewTask.CurrentTask = TEXT("Create realistic terrain with height variation");
                NewTask.Priority = TEXT("High");
                break;
            case 9: // Character Artist
                NewTask.CurrentTask = TEXT("Enhance TranspersonalCharacter with survival features");
                NewTask.Priority = TEXT("High");
                break;
            case 12: // Combat AI
                NewTask.CurrentTask = TEXT("Implement basic dinosaur behavior system");
                NewTask.Priority = TEXT("High");
                break;
            default:
                NewTask.CurrentTask = TEXT("Prepare systems for prototype phase");
                break;
        }
        
        AgentTasks.Add(NewTask);
    }
    
    ActiveAgentCount = AgentTasks.Num();
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline initialized with %d agents"), ActiveAgentCount);
}

void ADir_StudioDirector::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, float Progress)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.Status = NewStatus;
            Task.ProgressPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            
            UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s) status updated: %d, Progress: %.1f%%"), 
                   AgentID, *Task.AgentName, (int32)NewStatus, Progress);
            break;
        }
    }
    
    UpdateProductionMetrics();
}

void ADir_StudioDirector::AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const FString& Priority)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID)
        {
            Task.CurrentTask = TaskDescription;
            Task.Priority = Priority;
            Task.Status = EDir_AgentStatus::Working;
            
            UE_LOG(LogTemp, Warning, TEXT("Task assigned to Agent #%d: %s [Priority: %s]"), 
                   AgentID, *TaskDescription, *Priority);
            break;
        }
    }
}

bool ADir_StudioDirector::CheckMilestoneCompletion(const FString& MilestoneName)
{
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            return Milestone.bIsComplete;
        }
    }
    return false;
}

void ADir_StudioDirector::AdvanceProductionPhase()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::PrototypePhase;
            CurrentFocus = TEXT("Minimum Viable Playable Prototype");
            break;
        case EDir_ProductionPhase::PrototypePhase:
            CurrentPhase = EDir_ProductionPhase::ProductionPhase;
            CurrentFocus = TEXT("Full Feature Implementation");
            break;
        case EDir_ProductionPhase::ProductionPhase:
            CurrentPhase = EDir_ProductionPhase::PolishPhase;
            CurrentFocus = TEXT("Polish and Optimization");
            break;
        case EDir_ProductionPhase::PolishPhase:
            CurrentPhase = EDir_ProductionPhase::ReleaseReady;
            CurrentFocus = TEXT("Release Preparation");
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Production phase advanced. New focus: %s"), *CurrentFocus);
}

TArray<FDir_AgentTask> ADir_StudioDirector::GetActiveAgentTasks()
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

void ADir_StudioDirector::ResolveAgentBlocking(int32 AgentID, const FString& Resolution)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentID == AgentID && Task.Status == EDir_AgentStatus::Blocked)
        {
            Task.Status = EDir_AgentStatus::Working;
            BlockedAgents.RemoveSingle(Task.AgentName);
            
            UE_LOG(LogTemp, Warning, TEXT("Agent #%d unblocked. Resolution: %s"), AgentID, *Resolution);
            break;
        }
    }
}

float ADir_StudioDirector::CalculateOverallProgress()
{
    if (AgentTasks.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        TotalProgress += Task.ProgressPercentage;
    }
    
    return TotalProgress / AgentTasks.Num();
}

void ADir_StudioDirector::CreateMilestone(const FString& Name, const FString& Description, const TArray<int32>& RequiredAgents)
{
    FDir_ProductionMilestone NewMilestone;
    NewMilestone.MilestoneName = Name;
    NewMilestone.Description = Description;
    NewMilestone.RequiredAgents = RequiredAgents;
    NewMilestone.bIsComplete = false;
    NewMilestone.CompletionPercentage = 0.0f;
    
    ProductionMilestones.Add(NewMilestone);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone created: %s"), *Name);
}

void ADir_StudioDirector::CompleteMilestone(const FString& MilestoneName)
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.bIsComplete = true;
            Milestone.CompletionPercentage = 100.0f;
            
            UE_LOG(LogTemp, Warning, TEXT("MILESTONE COMPLETED: %s"), *MilestoneName);
            break;
        }
    }
}

void ADir_StudioDirector::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR PRODUCTION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Current Focus: %s"), *CurrentFocus);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), OverallProgress);
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), ActiveAgentCount);
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s): %s [%.1f%% - %d]"), 
               Task.AgentID, *Task.AgentName, *Task.CurrentTask, 
               Task.ProgressPercentage, (int32)Task.Status);
    }
}

void ADir_StudioDirector::ResetAllAgentTasks()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.ProgressPercentage = 0.0f;
        Task.CurrentTask = TEXT("Awaiting assignment");
    }
    
    BlockedAgents.Empty();
    CompletedTasks.Empty();
    OverallProgress = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("All agent tasks reset"));
}

void ADir_StudioDirector::SetupInitialMilestones()
{
    // Milestone 1: Walk Around Prototype
    TArray<int32> Milestone1Agents = {2, 3, 5, 9}; // Engine Architect, Core Systems, World Generator, Character Artist
    CreateMilestone(TEXT("Walk Around Prototype"), 
                   TEXT("Player can walk around on realistic terrain with basic survival HUD"), 
                   Milestone1Agents);
    
    // Milestone 2: Dinosaur Encounters
    TArray<int32> Milestone2Agents = {6, 10, 11, 12}; // Environment, Animation, NPC Behavior, Combat AI
    CreateMilestone(TEXT("Dinosaur Encounters"), 
                   TEXT("Dinosaurs with basic AI behavior and player interaction"), 
                   Milestone2Agents);
    
    // Milestone 3: Survival Mechanics
    TArray<int32> Milestone3Agents = {4, 15, 16, 17}; // Performance, Narrative, Audio, VFX
    CreateMilestone(TEXT("Survival Mechanics"), 
                   TEXT("Complete survival system with hunger, thirst, health, and environmental dangers"), 
                   Milestone3Agents);
}

void ADir_StudioDirector::ValidateAgentDependencies()
{
    // Check if agents have their required dependencies met
    for (FDir_AgentTask& Task : AgentTasks)
    {
        bool bAllDependenciesMet = true;
        
        for (const FString& Dependency : Task.Dependencies)
        {
            if (!CompletedTasks.Contains(Dependency))
            {
                bAllDependenciesMet = false;
                break;
            }
        }
        
        if (!bAllDependenciesMet && Task.Status == EDir_AgentStatus::Working)
        {
            Task.Status = EDir_AgentStatus::Blocked;
            BlockedAgents.AddUnique(Task.AgentName);
        }
    }
}

void ADir_StudioDirector::UpdateProductionMetrics()
{
    OverallProgress = CalculateOverallProgress();
    
    // Update milestone completion percentages
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (!Milestone.bIsComplete)
        {
            float MilestoneProgress = 0.0f;
            int32 ValidAgents = 0;
            
            for (int32 AgentID : Milestone.RequiredAgents)
            {
                for (const FDir_AgentTask& Task : AgentTasks)
                {
                    if (Task.AgentID == AgentID)
                    {
                        MilestoneProgress += Task.ProgressPercentage;
                        ValidAgents++;
                        break;
                    }
                }
            }
            
            if (ValidAgents > 0)
            {
                Milestone.CompletionPercentage = MilestoneProgress / ValidAgents;
                
                // Auto-complete milestone if all required agents are at 100%
                if (Milestone.CompletionPercentage >= 100.0f)
                {
                    CompleteMilestone(Milestone.MilestoneName);
                }
            }
        }
    }
}