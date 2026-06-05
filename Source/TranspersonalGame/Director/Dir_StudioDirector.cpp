#include "Dir_StudioDirector.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ADir_StudioDirector::ADir_StudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second
    
    // Set default values
    CurrentPhase = EDir_ProductionPhase::MinPlayable;
    CurrentCycleID = "PROD_CYCLE_AUTO_20260605_004";
    CycleNumber = 4;
    bIsInitialized = false;
}

void ADir_StudioDirector::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bIsInitialized)
    {
        InitializeAgentTasks();
        InitializeProductionMilestones();
        bIsInitialized = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Studio Director initialized - Cycle %d"), CycleNumber);
        LogProductionStatus();
    }
}

void ADir_StudioDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update milestone progress every 5 seconds
    if (LastUpdateTime >= 5.0f)
    {
        UpdateMilestoneProgress();
        LastUpdateTime = 0.0f;
    }
}

void ADir_StudioDirector::InitializeAgentTasks()
{
    AgentTasks.Empty();
    
    // Define all 19 agents with their current priorities
    TArray<TPair<FString, FString>> AgentDefinitions = {
        {TEXT("Studio Director"), TEXT("Coordinate production cycle and milestone tracking")},
        {TEXT("Engine Architect"), TEXT("Define technical architecture and UE5 integration patterns")},
        {TEXT("Core Systems Programmer"), TEXT("Implement physics, collision, and core gameplay systems")},
        {TEXT("Performance Optimizer"), TEXT("Ensure 60fps performance and optimize systems")},
        {TEXT("Procedural World Generator"), TEXT("Create terrain, biomes, and world structure")},
        {TEXT("Environment Artist"), TEXT("Populate world with vegetation, rocks, and props")},
        {TEXT("Architecture & Interior Agent"), TEXT("Build prehistoric structures and shelters")},
        {TEXT("Lighting & Atmosphere Agent"), TEXT("Implement day/night cycle and weather systems")},
        {TEXT("Character Artist Agent"), TEXT("Create player character and human NPCs")},
        {TEXT("Animation Agent"), TEXT("Implement character animations and motion systems")},
        {TEXT("NPC Behavior Agent"), TEXT("Define NPC AI and behavior patterns")},
        {TEXT("Combat & Enemy AI Agent"), TEXT("Implement dinosaur AI and combat systems")},
        {TEXT("Crowd & Traffic Simulation"), TEXT("Manage large-scale creature simulations")},
        {TEXT("Quest & Mission Designer"), TEXT("Create survival objectives and progression")},
        {TEXT("Narrative & Dialogue Agent"), TEXT("Write game lore and character dialogue")},
        {TEXT("Audio Agent"), TEXT("Implement adaptive music and sound effects")},
        {TEXT("VFX Agent"), TEXT("Create particle effects and visual feedback")},
        {TEXT("QA & Testing Agent"), TEXT("Test systems and validate gameplay")},
        {TEXT("Integration & Build Agent"), TEXT("Integrate all systems and manage builds")}
    };
    
    for (int32 i = 0; i < AgentDefinitions.Num(); i++)
    {
        FDir_AgentTask NewTask;
        NewTask.AgentID = i + 1;
        NewTask.AgentName = AgentDefinitions[i].Key;
        NewTask.CurrentTask = AgentDefinitions[i].Value;
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.ProgressPercent = 0.0f;
        NewTask.LastUpdate = FDateTime::Now();
        
        AgentTasks.Add(NewTask);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d agent tasks"), AgentTasks.Num());
}

void ADir_StudioDirector::InitializeProductionMilestones()
{
    ProductionMilestones.Empty();
    
    // Milestone 1: Walk Around (Current Priority)
    FDir_ProductionMilestone Milestone1;
    Milestone1.MilestoneName = TEXT("WALK_AROUND");
    Milestone1.Description = TEXT("Player can walk around in a basic world with terrain and dinosaurs");
    Milestone1.RequiredAgents = {2, 3, 5, 9, 10}; // Engine Architect, Core Systems, World Gen, Character, Animation
    Milestone1.RequiredDeliverables = {
        TEXT("ThirdPersonCharacter with WASD movement"),
        TEXT("Basic terrain with height variation"),
        TEXT("5 static dinosaur meshes in world"),
        TEXT("Camera system with boom and follow"),
        TEXT("Basic lighting and atmosphere")
    };
    Milestone1.bIsComplete = false;
    Milestone1.CompletionPercent = 15.0f; // Some progress made
    Milestone1.TargetDate = FDateTime::Now() + FTimespan::FromDays(2);
    ProductionMilestones.Add(Milestone1);
    
    // Milestone 2: Basic Survival
    FDir_ProductionMilestone Milestone2;
    Milestone2.MilestoneName = TEXT("BASIC_SURVIVAL");
    Milestone2.Description = TEXT("Player has health, hunger, thirst, and can interact with environment");
    Milestone2.RequiredAgents = {3, 11, 12, 14}; // Core Systems, NPC Behavior, Combat AI, Quest Designer
    Milestone2.RequiredDeliverables = {
        TEXT("Health/Hunger/Thirst system"),
        TEXT("Basic crafting (stone tools)"),
        TEXT("Resource gathering"),
        TEXT("Simple shelter building"),
        TEXT("Day/night survival mechanics")
    };
    Milestone2.bIsComplete = false;
    Milestone2.CompletionPercent = 0.0f;
    Milestone2.TargetDate = FDateTime::Now() + FTimespan::FromDays(7);
    ProductionMilestones.Add(Milestone2);
    
    // Milestone 3: Dinosaur Encounters
    FDir_ProductionMilestone Milestone3;
    Milestone3.MilestoneName = TEXT("DINOSAUR_ENCOUNTERS");
    Milestone3.Description = TEXT("Player can encounter and interact with dinosaurs (combat/stealth)");
    Milestone3.RequiredAgents = {12, 13, 16, 17}; // Combat AI, Crowd Simulation, Audio, VFX
    Milestone3.RequiredDeliverables = {
        TEXT("Dinosaur AI with territorial behavior"),
        TEXT("Combat system with primitive weapons"),
        TEXT("Stealth and hiding mechanics"),
        TEXT("Dinosaur sound design"),
        TEXT("Blood and impact VFX")
    };
    Milestone3.bIsComplete = false;
    Milestone3.CompletionPercent = 0.0f;
    Milestone3.TargetDate = FDateTime::Now() + FTimespan::FromDays(14);
    ProductionMilestones.Add(Milestone3);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d production milestones"), ProductionMilestones.Num());
}

void ADir_StudioDirector::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription)
{
    int32 AgentIndex = GetAgentIndex(AgentID);
    if (AgentIndex != INDEX_NONE)
    {
        AgentTasks[AgentIndex].Status = NewStatus;
        if (!TaskDescription.IsEmpty())
        {
            AgentTasks[AgentIndex].CurrentTask = TaskDescription;
        }
        AgentTasks[AgentIndex].LastUpdate = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Agent %d (%s) status updated to %d: %s"), 
               AgentID, *AgentTasks[AgentIndex].AgentName, (int32)NewStatus, *TaskDescription);
    }
}

void ADir_StudioDirector::AddAgentDeliverable(int32 AgentID, const FString& Deliverable)
{
    int32 AgentIndex = GetAgentIndex(AgentID);
    if (AgentIndex != INDEX_NONE)
    {
        AgentTasks[AgentIndex].Deliverables.AddUnique(Deliverable);
        AgentTasks[AgentIndex].LastUpdate = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Agent %d deliverable added: %s"), AgentID, *Deliverable);
    }
}

FDir_AgentTask ADir_StudioDirector::GetAgentTask(int32 AgentID) const
{
    int32 AgentIndex = GetAgentIndex(AgentID);
    if (AgentIndex != INDEX_NONE)
    {
        return AgentTasks[AgentIndex];
    }
    return FDir_AgentTask();
}

TArray<int32> ADir_StudioDirector::GetBlockedAgents() const
{
    TArray<int32> BlockedAgents;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedAgents.Add(Task.AgentID);
        }
    }
    return BlockedAgents;
}

void ADir_StudioDirector::UpdateMilestoneProgress()
{
    for (FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.bIsComplete) continue;
        
        float TotalProgress = 0.0f;
        int32 ValidAgents = 0;
        
        // Calculate progress based on required agents
        for (int32 AgentID : Milestone.RequiredAgents)
        {
            int32 AgentIndex = GetAgentIndex(AgentID);
            if (AgentIndex != INDEX_NONE)
            {
                TotalProgress += AgentTasks[AgentIndex].ProgressPercent;
                ValidAgents++;
            }
        }
        
        if (ValidAgents > 0)
        {
            Milestone.CompletionPercent = TotalProgress / ValidAgents;
            
            // Mark as complete if 95% or higher
            if (Milestone.CompletionPercent >= 95.0f && !Milestone.bIsComplete)
            {
                Milestone.bIsComplete = true;
                Milestone.CompletedDate = FDateTime::Now();
                
                UE_LOG(LogTemp, Warning, TEXT("MILESTONE COMPLETED: %s"), *Milestone.MilestoneName);
            }
        }
    }
}

bool ADir_StudioDirector::IsMilestoneComplete(const FString& MilestoneName) const
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

float ADir_StudioDirector::GetOverallProgress() const
{
    if (ProductionMilestones.Num() == 0) return 0.0f;
    
    float TotalProgress = 0.0f;
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        TotalProgress += Milestone.CompletionPercent;
    }
    
    return TotalProgress / ProductionMilestones.Num();
}

void ADir_StudioDirector::StartNewCycle(const FString& CycleID)
{
    CurrentCycleID = CycleID;
    CycleNumber++;
    
    // Reset agent progress for new cycle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status != EDir_AgentStatus::Complete)
        {
            Task.Status = EDir_AgentStatus::Working;
        }
        Task.LastUpdate = FDateTime::Now();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Started new production cycle: %s (Cycle %d)"), *CycleID, CycleNumber);
}

void ADir_StudioDirector::CompleteCycle()
{
    UpdateMilestoneProgress();
    
    // Log cycle completion
    FString Report = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("Cycle %d Complete:\n%s"), CycleNumber, *Report);
}

FString ADir_StudioDirector::GenerateProductionReport() const
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT - CYCLE %d ===\n"), CycleNumber);
    Report += FString::Printf(TEXT("Phase: %s\n"), *UEnum::GetValueAsString(CurrentPhase));
    Report += FString::Printf(TEXT("Overall Progress: %.1f%%\n\n"), GetOverallProgress());
    
    // Milestone status
    Report += TEXT("MILESTONES:\n");
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        FString Status = Milestone.bIsComplete ? TEXT("COMPLETE") : TEXT("IN PROGRESS");
        Report += FString::Printf(TEXT("- %s: %s (%.1f%%)\n"), 
                                  *Milestone.MilestoneName, *Status, Milestone.CompletionPercent);
    }
    
    // Agent status summary
    Report += TEXT("\nAGENT STATUS:\n");
    int32 WorkingAgents = 0, BlockedAgents = 0, CompleteAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Working: WorkingAgents++; break;
            case EDir_AgentStatus::Blocked: BlockedAgents++; break;
            case EDir_AgentStatus::Complete: CompleteAgents++; break;
        }
    }
    
    Report += FString::Printf(TEXT("Working: %d, Blocked: %d, Complete: %d\n"), 
                              WorkingAgents, BlockedAgents, CompleteAgents);
    
    return Report;
}

void ADir_StudioDirector::LogProductionStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== STUDIO DIRECTOR STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %s"), *UEnum::GetValueAsString(CurrentPhase));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %s (#%d)"), *CurrentCycleID, CycleNumber);
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), GetOverallProgress());
    UE_LOG(LogTemp, Warning, TEXT("Active Agents: %d"), AgentTasks.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active Milestones: %d"), ProductionMilestones.Num());
}

void ADir_StudioDirector::DebugPrintAllAgentStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ALL AGENT STATUS ==="));
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        FString StatusStr = UEnum::GetValueAsString(Task.Status);
        UE_LOG(LogTemp, Warning, TEXT("Agent %d (%s): %s - %s (%.1f%%)"), 
               Task.AgentID, *Task.AgentName, *StatusStr, *Task.CurrentTask, Task.ProgressPercent);
    }
}

void ADir_StudioDirector::DebugResetAllAgents()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        Task.Status = EDir_AgentStatus::Idle;
        Task.ProgressPercent = 0.0f;
        Task.BlockingIssue = TEXT("");
        Task.Deliverables.Empty();
        Task.LastUpdate = FDateTime::Now();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("All agents reset to idle state"));
}

void ADir_StudioDirector::DebugSimulateAgentProgress()
{
    // Simulate some progress for testing
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle)
        {
            Task.Status = EDir_AgentStatus::Working;
            Task.ProgressPercent = FMath::RandRange(10.0f, 40.0f);
        }
    }
    
    UpdateMilestoneProgress();
    UE_LOG(LogTemp, Warning, TEXT("Simulated agent progress"));
}

int32 ADir_StudioDirector::GetAgentIndex(int32 AgentID) const
{
    for (int32 i = 0; i < AgentTasks.Num(); i++)
    {
        if (AgentTasks[i].AgentID == AgentID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void ADir_StudioDirector::ValidateAgentChain() const
{
    // Validate that all required agents exist and are properly configured
    TArray<int32> RequiredAgents = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    
    for (int32 RequiredID : RequiredAgents)
    {
        bool bFound = false;
        for (const FDir_AgentTask& Task : AgentTasks)
        {
            if (Task.AgentID == RequiredID)
            {
                bFound = true;
                break;
            }
        }
        
        if (!bFound)
        {
            UE_LOG(LogTemp, Error, TEXT("Missing required agent: %d"), RequiredID);
        }
    }
}

void ADir_StudioDirector::CheckMilestoneDependencies()
{
    // Check if milestone dependencies are met
    for (const FDir_ProductionMilestone& Milestone : ProductionMilestones)
    {
        if (Milestone.bIsComplete) continue;
        
        bool bAllDependenciesMet = true;
        for (int32 AgentID : Milestone.RequiredAgents)
        {
            int32 AgentIndex = GetAgentIndex(AgentID);
            if (AgentIndex == INDEX_NONE || AgentTasks[AgentIndex].Status == EDir_AgentStatus::Blocked)
            {
                bAllDependenciesMet = false;
                break;
            }
        }
        
        if (!bAllDependenciesMet)
        {
            UE_LOG(LogTemp, Warning, TEXT("Milestone %s has unmet dependencies"), *Milestone.MilestoneName);
        }
    }
}