#include "Dir_ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"

ADir_ProductionCoordinator::ADir_ProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    LastUpdateTime = 0.0f;
    CompletedMilestones = 0;
}

void ADir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAgents();
    InitializeMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized with %d agents and %d milestones"), 
           Agents.Num(), Milestones.Num());
}

void ADir_ProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update every 5 seconds
    if (LastUpdateTime >= 5.0f)
    {
        CheckForBlockedAgents();
        ValidateProductionChain();
        LastUpdateTime = 0.0f;
    }
}

void ADir_ProductionCoordinator::InitializeAgents()
{
    Agents.Empty();
    
    // Initialize all 19 agents
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
    
    for (int32 i = 0; i < AgentNames.Num(); i++)
    {
        FDir_AgentInfo NewAgent;
        NewAgent.AgentID = i + 1;
        NewAgent.AgentName = AgentNames[i];
        NewAgent.Status = EDir_AgentStatus::Idle;
        NewAgent.CurrentTask = TEXT("Awaiting assignment");
        NewAgent.ProgressPercent = 0.0f;
        NewAgent.LastUpdate = FDateTime::Now();
        
        Agents.Add(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d agents"), Agents.Num());
}

void ADir_ProductionCoordinator::InitializeMilestones()
{
    Milestones.Empty();
    
    // Milestone 1: Character Movement
    FDir_ProductionMilestone M1;
    M1.Type = EDir_MilestoneType::CharacterMovement;
    M1.Name = TEXT("Character Movement System");
    M1.Description = TEXT("Player can walk, run, jump with responsive controls");
    M1.bIsComplete = false;
    M1.RequiredAgents = {2, 3, 9, 10}; // Core Systems, Performance, Character Artist, Animation
    M1.WorldLocation = FVector(0, 0, 200);
    Milestones.Add(M1);
    
    // Milestone 2: Dinosaur AI
    FDir_ProductionMilestone M2;
    M2.Type = EDir_MilestoneType::DinosaurAI;
    M2.Name = TEXT("Dinosaur AI System");
    M2.Description = TEXT("Dinosaurs with basic behavior, collision, and interaction");
    M2.bIsComplete = false;
    M2.RequiredAgents = {11, 12, 13}; // NPC Behavior, Combat AI, Crowd Simulation
    M2.WorldLocation = FVector(1000, 0, 200);
    Milestones.Add(M2);
    
    // Milestone 3: Survival Systems
    FDir_ProductionMilestone M3;
    M3.Type = EDir_MilestoneType::SurvivalSystems;
    M3.Name = TEXT("Survival Systems");
    M3.Description = TEXT("Health, hunger, thirst, stamina with UI display");
    M3.bIsComplete = false;
    M3.RequiredAgents = {3, 14}; // Core Systems, Quest Designer
    M3.WorldLocation = FVector(0, 1000, 200);
    Milestones.Add(M3);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized %d milestones"), Milestones.Num());
}

void ADir_ProductionCoordinator::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& Task)
{
    for (FDir_AgentInfo& Agent : Agents)
    {
        if (Agent.AgentID == AgentID)
        {
            Agent.Status = NewStatus;
            Agent.CurrentTask = Task;
            Agent.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("Agent %d (%s) status updated: %s - %s"), 
                   AgentID, *Agent.AgentName, 
                   *UEnum::GetValueAsString(NewStatus), *Task);
            break;
        }
    }
}

void ADir_ProductionCoordinator::CompleteMilestone(EDir_MilestoneType MilestoneType)
{
    for (FDir_ProductionMilestone& Milestone : Milestones)
    {
        if (Milestone.Type == MilestoneType && !Milestone.bIsComplete)
        {
            if (CheckMilestoneRequirements(MilestoneType))
            {
                Milestone.bIsComplete = true;
                CompletedMilestones++;
                
                UE_LOG(LogTemp, Warning, TEXT("MILESTONE COMPLETED: %s"), *Milestone.Name);
                
                // Notify all agents
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        FString::Printf(TEXT("MILESTONE COMPLETED: %s"), *Milestone.Name));
                }
            }
            break;
        }
    }
}

bool ADir_ProductionCoordinator::CheckMilestoneRequirements(EDir_MilestoneType MilestoneType)
{
    for (const FDir_ProductionMilestone& Milestone : Milestones)
    {
        if (Milestone.Type == MilestoneType)
        {
            // Check if all required agents have completed their tasks
            for (int32 RequiredAgentID : Milestone.RequiredAgents)
            {
                bool bAgentComplete = false;
                for (const FDir_AgentInfo& Agent : Agents)
                {
                    if (Agent.AgentID == RequiredAgentID && Agent.Status == EDir_AgentStatus::Complete)
                    {
                        bAgentComplete = true;
                        break;
                    }
                }
                
                if (!bAgentComplete)
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

float ADir_ProductionCoordinator::GetOverallProgress()
{
    if (Milestones.Num() == 0) return 0.0f;
    
    return (float)CompletedMilestones / (float)Milestones.Num() * 100.0f;
}

TArray<FDir_AgentInfo> ADir_ProductionCoordinator::GetAgentsByStatus(EDir_AgentStatus Status)
{
    TArray<FDir_AgentInfo> FilteredAgents;
    
    for (const FDir_AgentInfo& Agent : Agents)
    {
        if (Agent.Status == Status)
        {
            FilteredAgents.Add(Agent);
        }
    }
    
    return FilteredAgents;
}

void ADir_ProductionCoordinator::LogProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Progress: %.1f%%"), GetOverallProgress());
    UE_LOG(LogTemp, Warning, TEXT("Completed Milestones: %d/%d"), CompletedMilestones, Milestones.Num());
    
    // Count agents by status
    int32 IdleCount = GetAgentsByStatus(EDir_AgentStatus::Idle).Num();
    int32 WorkingCount = GetAgentsByStatus(EDir_AgentStatus::Working).Num();
    int32 CompleteCount = GetAgentsByStatus(EDir_AgentStatus::Complete).Num();
    int32 BlockedCount = GetAgentsByStatus(EDir_AgentStatus::Blocked).Num();
    int32 ErrorCount = GetAgentsByStatus(EDir_AgentStatus::Error).Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Agent Status: Idle=%d, Working=%d, Complete=%d, Blocked=%d, Error=%d"),
           IdleCount, WorkingCount, CompleteCount, BlockedCount, ErrorCount);
}

bool ADir_ProductionCoordinator::ValidateCharacterMovement()
{
    // Check if TranspersonalCharacter exists and has movement
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Look for player character in the world
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && Pawn->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            return true;
        }
    }
    
    return false;
}

bool ADir_ProductionCoordinator::ValidateDinosaurAI()
{
    // Check for dinosaur actors with AI components
    UWorld* World = GetWorld();
    if (!World) return false;
    
    int32 DinosaurCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetName().Contains(TEXT("Dinosaur")) || 
                     Actor->GetName().Contains(TEXT("TRex")) ||
                     Actor->GetName().Contains(TEXT("Raptor"))))
        {
            DinosaurCount++;
        }
    }
    
    return DinosaurCount >= 3; // At least 3 dinosaurs
}

bool ADir_ProductionCoordinator::ValidateSurvivalSystems()
{
    // Check if survival components exist
    // This would check for health/hunger/thirst systems
    return false; // Placeholder - needs actual survival system validation
}

void ADir_ProductionCoordinator::DebugPrintAgentStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT STATUS DEBUG ==="));
    for (const FDir_AgentInfo& Agent : Agents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent %d: %s - %s (%s)"), 
               Agent.AgentID, *Agent.AgentName, 
               *UEnum::GetValueAsString(Agent.Status), *Agent.CurrentTask);
    }
}

void ADir_ProductionCoordinator::DebugPrintMilestones()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MILESTONE STATUS DEBUG ==="));
    for (const FDir_ProductionMilestone& Milestone : Milestones)
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone: %s - %s"), 
               *Milestone.Name, Milestone.bIsComplete ? TEXT("COMPLETE") : TEXT("PENDING"));
    }
}

void ADir_ProductionCoordinator::CheckForBlockedAgents()
{
    FDateTime CurrentTime = FDateTime::Now();
    
    for (FDir_AgentInfo& Agent : Agents)
    {
        // If agent has been working for more than 10 minutes, mark as potentially blocked
        if (Agent.Status == EDir_AgentStatus::Working)
        {
            FTimespan TimeDiff = CurrentTime - Agent.LastUpdate;
            if (TimeDiff.GetTotalMinutes() > 10.0)
            {
                Agent.Status = EDir_AgentStatus::Blocked;
                UE_LOG(LogTemp, Error, TEXT("Agent %d (%s) marked as BLOCKED - no update for %.1f minutes"),
                       Agent.AgentID, *Agent.AgentName, TimeDiff.GetTotalMinutes());
            }
        }
    }
}

void ADir_ProductionCoordinator::ValidateProductionChain()
{
    // Validate that agents are working in proper dependency order
    // Engine Architect (2) should complete before Core Systems (3)
    // Core Systems (3) should complete before Performance (4), etc.
    
    bool bArchitectComplete = false;
    bool bCoreSystemsComplete = false;
    
    for (const FDir_AgentInfo& Agent : Agents)
    {
        if (Agent.AgentID == 2 && Agent.Status == EDir_AgentStatus::Complete)
            bArchitectComplete = true;
        if (Agent.AgentID == 3 && Agent.Status == EDir_AgentStatus::Complete)
            bCoreSystemsComplete = true;
    }
    
    // Log warnings if dependency chain is broken
    if (bCoreSystemsComplete && !bArchitectComplete)
    {
        UE_LOG(LogTemp, Warning, TEXT("DEPENDENCY WARNING: Core Systems complete but Engine Architect not complete"));
    }
}