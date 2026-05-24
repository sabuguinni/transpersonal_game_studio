#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260522_002");
    TotalCyclesCompleted = 0;
    LastUpdateTime = 0.0f;
    bPipelineInitialized = false;
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionPipeline();
    CreateWalkAroundMilestone();
    CreatePlayablePrototypeMilestone();
    
    UE_LOG(LogTemp, Warning, TEXT("Studio Director Production Coordinator initialized - Cycle: %s"), *CurrentCycleID);
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update every 5 seconds
    if (LastUpdateTime >= 5.0f)
    {
        ValidateGameplayElements();
        LastUpdateTime = 0.0f;
    }
}

void AProductionCoordinator::InitializeProductionPipeline()
{
    if (bPipelineInitialized)
        return;
        
    AgentTasks.Empty();
    
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
        FDir_AgentTask NewTask;
        NewTask.AgentNumber = i + 1;
        NewTask.AgentName = AgentNames[i];
        NewTask.Status = EDir_AgentStatus::Idle;
        NewTask.ProgressPercent = 0.0f;
        NewTask.LastUpdate = FDateTime::Now();
        
        AgentTasks.Add(NewTask);
    }
    
    SetupAgentHierarchy();
    bPipelineInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Production pipeline initialized with %d agents"), AgentTasks.Num());
}

void AProductionCoordinator::SetupAgentHierarchy()
{
    // Set initial tasks for key agents
    if (FDir_AgentTask* EngineArchitect = FindAgentTask(2))
    {
        EngineArchitect->CurrentTask = TEXT("Define core architecture and compilation rules");
        EngineArchitect->Status = EDir_AgentStatus::Working;
    }
    
    if (FDir_AgentTask* CoreSystems = FindAgentTask(3))
    {
        CoreSystems->CurrentTask = TEXT("Implement physics, collision, and movement systems");
        CoreSystems->Status = EDir_AgentStatus::Idle; // Waiting for architect
    }
    
    if (FDir_AgentTask* WorldGen = FindAgentTask(5))
    {
        WorldGen->CurrentTask = TEXT("Generate terrain and biomes for MinPlayableMap");
        WorldGen->Status = EDir_AgentStatus::Working;
    }
}

void AProductionCoordinator::UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, float Progress)
{
    if (FDir_AgentTask* Task = FindAgentTask(AgentNumber))
    {
        Task->Status = NewStatus;
        Task->ProgressPercent = Progress;
        Task->LastUpdate = FDateTime::Now();
        
        UE_LOG(LogTemp, Log, TEXT("Agent #%d (%s) status updated: %d, Progress: %.1f%%"), 
               AgentNumber, *Task->AgentName, (int32)NewStatus, Progress);
    }
}

void AProductionCoordinator::AddAgentDeliverable(int32 AgentNumber, const FString& Deliverable)
{
    if (FDir_AgentTask* Task = FindAgentTask(AgentNumber))
    {
        Task->Deliverables.Add(Deliverable);
        UE_LOG(LogTemp, Log, TEXT("Agent #%d deliverable added: %s"), AgentNumber, *Deliverable);
    }
}

void AProductionCoordinator::CreateWalkAroundMilestone()
{
    FDir_ProductionMilestone WalkAround;
    WalkAround.MilestoneName = TEXT("Walk Around Prototype");
    WalkAround.Phase = EDir_MilestonePhase::Development;
    WalkAround.RequiredAgents = {1, 3, 5, 9, 10}; // Director, Core, World, Character, Animation
    
    WalkAround.Criteria.Add(TEXT("ThirdPersonCharacter with WASD movement"));
    WalkAround.Criteria.Add(TEXT("Camera boom + follow camera"));
    WalkAround.Criteria.Add(TEXT("Landscape with basic terrain"));
    WalkAround.Criteria.Add(TEXT("Player can walk, run, jump"));
    WalkAround.Criteria.Add(TEXT("3-5 static dinosaur meshes in world"));
    WalkAround.Criteria.Add(TEXT("Directional light + sky + fog"));
    
    WalkAround.TargetDate = FDateTime::Now() + FTimespan::FromDays(1);
    
    Milestones.Add(WalkAround);
}

void AProductionCoordinator::CreatePlayablePrototypeMilestone()
{
    FDir_ProductionMilestone Prototype;
    Prototype.MilestoneName = TEXT("Minimum Viable Playable Prototype");
    Prototype.Phase = EDir_MilestonePhase::Development;
    Prototype.RequiredAgents = {1, 2, 3, 5, 6, 9, 10, 11, 12}; // Core team
    
    Prototype.Criteria.Add(TEXT("Character movement with survival stats"));
    Prototype.Criteria.Add(TEXT("5 biomes with populated terrain"));
    Prototype.Criteria.Add(TEXT("Real dinosaur actors with basic AI"));
    Prototype.Criteria.Add(TEXT("Survival HUD with health/hunger/thirst"));
    Prototype.Criteria.Add(TEXT("Basic crafting system"));
    Prototype.Criteria.Add(TEXT("Day/night cycle"));
    
    Prototype.TargetDate = FDateTime::Now() + FTimespan::FromDays(3);
    
    Milestones.Add(Prototype);
}

bool AProductionCoordinator::ValidateMilestoneCompletion(const FString& MilestoneName)
{
    for (FDir_ProductionMilestone& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            // Check if all required agents have completed their tasks
            bool bAllAgentsComplete = true;
            for (int32 AgentNum : Milestone.RequiredAgents)
            {
                if (FDir_AgentTask* Task = FindAgentTask(AgentNum))
                {
                    if (Task->Status != EDir_AgentStatus::Completed)
                    {
                        bAllAgentsComplete = false;
                        break;
                    }
                }
            }
            
            if (bAllAgentsComplete && CheckMinimumViablePrototype())
            {
                Milestone.bIsComplete = true;
                Milestone.Phase = EDir_MilestonePhase::Complete;
                UE_LOG(LogTemp, Warning, TEXT("Milestone completed: %s"), *MilestoneName);
                return true;
            }
        }
    }
    
    return false;
}

bool AProductionCoordinator::CheckMinimumViablePrototype()
{
    // Check for essential gameplay elements
    int32 ActorCount = CountVisibleActorsInLevel();
    bool bHasCharacter = false;
    bool bHasDinosaurs = false;
    
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Check for player character
    for (TActorIterator<APawn> ActorItr(World); ActorItr; ++ActorItr)
    {
        APawn* Pawn = *ActorItr;
        if (Pawn && Pawn->IsA<ACharacter>())
        {
            bHasCharacter = true;
            break;
        }
    }
    
    // Check for dinosaur actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
                ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("tricerat")))
            {
                bHasDinosaurs = true;
                break;
            }
        }
    }
    
    bool bIsViable = (ActorCount > 10) && bHasCharacter && bHasDinosaurs;
    
    UE_LOG(LogTemp, Warning, TEXT("MVP Check - Actors: %d, Character: %s, Dinosaurs: %s, Viable: %s"),
           ActorCount, bHasCharacter ? TEXT("YES") : TEXT("NO"), 
           bHasDinosaurs ? TEXT("YES") : TEXT("NO"), bIsViable ? TEXT("YES") : TEXT("NO"));
    
    return bIsViable;
}

bool AProductionCoordinator::ValidateCompilation()
{
    // This would be called by the build system
    // For now, assume compilation is valid if we can create actors
    UWorld* World = GetWorld();
    return World != nullptr;
}

bool AProductionCoordinator::ValidateGameplayElements()
{
    return CheckMinimumViablePrototype();
}

int32 AProductionCoordinator::CountVisibleActorsInLevel()
{
    int32 Count = 0;
    UWorld* World = GetWorld();
    
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsHidden() && Actor->GetRootComponent())
            {
                Count++;
            }
        }
    }
    
    return Count;
}

void AProductionCoordinator::DispatchTaskToNextAgent()
{
    // Find next agent in sequence that needs work
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Idle)
        {
            Task.Status = EDir_AgentStatus::Working;
            Task.LastUpdate = FDateTime::Now();
            
            UE_LOG(LogTemp, Warning, TEXT("Dispatching task to Agent #%d (%s)"), 
                   Task.AgentNumber, *Task.AgentName);
            break;
        }
    }
}

FString AProductionCoordinator::GenerateProductionReport()
{
    FString Report = FString::Printf(TEXT("=== PRODUCTION REPORT - %s ===\n"), *CurrentCycleID);
    
    int32 CompletedAgents = 0;
    int32 WorkingAgents = 0;
    int32 BlockedAgents = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Completed: CompletedAgents++; break;
            case EDir_AgentStatus::Working: WorkingAgents++; break;
            case EDir_AgentStatus::Blocked: BlockedAgents++; break;
        }
    }
    
    Report += FString::Printf(TEXT("Agents - Completed: %d, Working: %d, Blocked: %d\n"), 
                             CompletedAgents, WorkingAgents, BlockedAgents);
    
    Report += FString::Printf(TEXT("Total Actors in Level: %d\n"), CountVisibleActorsInLevel());
    Report += FString::Printf(TEXT("MVP Status: %s\n"), CheckMinimumViablePrototype() ? TEXT("VIABLE") : TEXT("NOT VIABLE"));
    
    return Report;
}

void AProductionCoordinator::LogCycleCompletion()
{
    TotalCyclesCompleted++;
    
    FString Report = GenerateProductionReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    // Reset for next cycle
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            Task.Status = EDir_AgentStatus::Idle;
            Task.ProgressPercent = 0.0f;
        }
    }
}

FDir_AgentTask* AProductionCoordinator::FindAgentTask(int32 AgentNumber)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber)
        {
            return &Task;
        }
    }
    return nullptr;
}