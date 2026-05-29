#include "Director_ProductionPipeline.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionPipeline::UDir_ProductionPipeline()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    CurrentPhase = EDir_ProductionPhase::CoreSystems;
    CurrentCycleNumber = 0;
    TargetCycleTime = 300.0f; // 5 minutes per cycle
    CycleStartTime = 0.0f;
    
    // Initialize critical milestone flags
    bCharacterMovementComplete = false;
    bSurvivalSystemComplete = false;
    bDinosaurAIComplete = false;
    bTerrainSystemComplete = false;
    
    // Initialize agent names
    AgentNames = {
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
}

void UDir_ProductionPipeline::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProductionChain();
    CycleStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Pipeline initialized - Phase: %d, Cycle: %d"), 
           static_cast<int32>(CurrentPhase), CurrentCycleNumber);
}

void UDir_ProductionPipeline::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateProductionMetrics();
    
    // Check for cycle timeout
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - CycleStartTime > TargetCycleTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cycle %d timeout - advancing to next cycle"), CurrentCycleNumber);
        CurrentCycleNumber++;
        CycleStartTime = CurrentTime;
    }
}

void UDir_ProductionPipeline::InitializeProductionChain()
{
    // Initialize agent statuses
    for (int32 i = 1; i <= 19; ++i)
    {
        AgentStatuses.Add(i, EDir_AgentStatus::Idle);
    }
    
    // Set up agent dependencies
    AgentDependencies.Add(2, {1}); // Engine Architect depends on Studio Director
    AgentDependencies.Add(3, {2}); // Core Systems depends on Engine Architect
    AgentDependencies.Add(4, {3}); // Performance depends on Core Systems
    AgentDependencies.Add(5, {2, 3}); // World Generator depends on Engine + Core
    AgentDependencies.Add(6, {5}); // Environment depends on World Generator
    AgentDependencies.Add(7, {6}); // Architecture depends on Environment
    AgentDependencies.Add(8, {7}); // Lighting depends on Architecture
    AgentDependencies.Add(9, {2}); // Character Artist depends on Engine Architect
    AgentDependencies.Add(10, {9}); // Animation depends on Character Artist
    AgentDependencies.Add(11, {10}); // NPC Behavior depends on Animation
    AgentDependencies.Add(12, {11}); // Combat AI depends on NPC Behavior
    AgentDependencies.Add(13, {12}); // Crowd Simulation depends on Combat AI
    AgentDependencies.Add(14, {15}); // Quest Designer depends on Narrative
    AgentDependencies.Add(15, {1}); // Narrative depends on Studio Director
    AgentDependencies.Add(16, {8}); // Audio depends on Lighting (world complete)
    AgentDependencies.Add(17, {16}); // VFX depends on Audio
    AgentDependencies.Add(18, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17}); // QA depends on all
    AgentDependencies.Add(19, {18}); // Integration depends on QA
    
    // Initialize critical milestones
    FDir_Milestone CharacterMilestone;
    CharacterMilestone.Name = TEXT("Character Movement");
    CharacterMilestone.Description = TEXT("Player can walk, run, jump with WASD controls");
    CharacterMilestone.RequiredAgents = {3, 9, 10};
    CharacterMilestone.Priority = 10.0f;
    RegisterMilestone(CharacterMilestone);
    
    FDir_Milestone SurvivalMilestone;
    SurvivalMilestone.Name = TEXT("Survival System");
    SurvivalMilestone.Description = TEXT("Health, hunger, thirst, stamina mechanics");
    SurvivalMilestone.RequiredAgents = {3, 9};
    SurvivalMilestone.Priority = 9.0f;
    RegisterMilestone(SurvivalMilestone);
    
    FDir_Milestone DinosaurMilestone;
    DinosaurMilestone.Name = TEXT("Dinosaur AI");
    DinosaurMilestone.Description = TEXT("T-Rex with basic pursue behavior");
    DinosaurMilestone.RequiredAgents = {11, 12};
    DinosaurMilestone.Priority = 8.0f;
    RegisterMilestone(DinosaurMilestone);
    
    FDir_Milestone TerrainMilestone;
    TerrainMilestone.Name = TEXT("Terrain System");
    TerrainMilestone.Description = TEXT("Landscape with height variation and collision");
    TerrainMilestone.RequiredAgents = {5, 6};
    TerrainMilestone.Priority = 7.0f;
    RegisterMilestone(TerrainMilestone);
}

void UDir_ProductionPipeline::AdvanceToNextPhase()
{
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::PreProduction:
            CurrentPhase = EDir_ProductionPhase::CoreSystems;
            break;
        case EDir_ProductionPhase::CoreSystems:
            if (ValidateMilestone(TEXT("Character Movement")))
            {
                CurrentPhase = EDir_ProductionPhase::WorldGeneration;
            }
            break;
        case EDir_ProductionPhase::WorldGeneration:
            if (ValidateMilestone(TEXT("Terrain System")))
            {
                CurrentPhase = EDir_ProductionPhase::CharacterSystems;
            }
            break;
        case EDir_ProductionPhase::CharacterSystems:
            if (ValidateMilestone(TEXT("Survival System")) && ValidateMilestone(TEXT("Dinosaur AI")))
            {
                CurrentPhase = EDir_ProductionPhase::GameplayMechanics;
            }
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Advanced to production phase: %d"), static_cast<int32>(CurrentPhase));
}

bool UDir_ProductionPipeline::ValidateMilestone(const FString& MilestoneName)
{
    for (FDir_Milestone& Milestone : Milestones)
    {
        if (Milestone.Name == MilestoneName)
        {
            if (!Milestone.bIsComplete && CheckMilestoneDependencies(Milestone))
            {
                // Validate specific milestones
                if (MilestoneName == TEXT("Character Movement"))
                {
                    Milestone.bIsComplete = ValidateCharacterMovement();
                    bCharacterMovementComplete = Milestone.bIsComplete;
                }
                else if (MilestoneName == TEXT("Survival System"))
                {
                    // Check for SurvivalComponent existence
                    Milestone.bIsComplete = true; // Placeholder - should check actual component
                    bSurvivalSystemComplete = Milestone.bIsComplete;
                }
                else if (MilestoneName == TEXT("Dinosaur AI"))
                {
                    Milestone.bIsComplete = ValidateDinosaurAI();
                    bDinosaurAIComplete = Milestone.bIsComplete;
                }
                else if (MilestoneName == TEXT("Terrain System"))
                {
                    Milestone.bIsComplete = ValidateTerrainSystem();
                    bTerrainSystemComplete = Milestone.bIsComplete;
                }
            }
            return Milestone.bIsComplete;
        }
    }
    return false;
}

void UDir_ProductionPipeline::AssignTaskToAgent(int32 AgentID, const FDir_AgentTask& Task)
{
    if (AgentID >= 1 && AgentID <= 19)
    {
        ActiveTasks.Add(Task);
        AgentStatuses[AgentID] = EDir_AgentStatus::Working;
        
        UE_LOG(LogTemp, Log, TEXT("Assigned task '%s' to Agent #%d: %s"), 
               *Task.TaskName, AgentID, *AgentNames[AgentID - 1]);
    }
}

void UDir_ProductionPipeline::UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus)
{
    if (AgentStatuses.Contains(AgentID))
    {
        AgentStatuses[AgentID] = NewStatus;
        
        if (NewStatus == EDir_AgentStatus::Complete)
        {
            Metrics.CompletedTasks++;
        }
        else if (NewStatus == EDir_AgentStatus::Error)
        {
            FString ErrorMsg = FString::Printf(TEXT("Agent #%d (%s) encountered error"), 
                                               AgentID, *AgentNames[AgentID - 1]);
            Metrics.CriticalBlocks.Add(ErrorMsg);
            TriggerEmergencyProtocol(ErrorMsg);
        }
    }
}

TArray<FDir_AgentTask> UDir_ProductionPipeline::GetCriticalPath()
{
    TArray<FDir_AgentTask> CriticalTasks;
    
    // Priority order based on current phase
    TArray<int32> CriticalAgents;
    
    switch (CurrentPhase)
    {
        case EDir_ProductionPhase::CoreSystems:
            CriticalAgents = {1, 2, 3, 9}; // Director, Architect, Core, Character
            break;
        case EDir_ProductionPhase::WorldGeneration:
            CriticalAgents = {5, 6}; // World Generator, Environment
            break;
        case EDir_ProductionPhase::CharacterSystems:
            CriticalAgents = {10, 11, 12}; // Animation, NPC, Combat AI
            break;
        default:
            CriticalAgents = {1, 2, 3}; // Fallback to core agents
            break;
    }
    
    for (const FDir_AgentTask& Task : ActiveTasks)
    {
        if (CriticalAgents.Contains(Task.AgentID))
        {
            CriticalTasks.Add(Task);
        }
    }
    
    return CriticalTasks;
}

void UDir_ProductionPipeline::RegisterMilestone(const FDir_Milestone& Milestone)
{
    Milestones.Add(Milestone);
    UE_LOG(LogTemp, Log, TEXT("Registered milestone: %s"), *Milestone.Name);
}

void UDir_ProductionPipeline::CompleteMilestone(const FString& MilestoneName)
{
    for (FDir_Milestone& Milestone : Milestones)
    {
        if (Milestone.Name == MilestoneName)
        {
            Milestone.bIsComplete = true;
            UE_LOG(LogTemp, Warning, TEXT("Completed milestone: %s"), *MilestoneName);
            break;
        }
    }
}

TArray<FDir_Milestone> UDir_ProductionPipeline::GetPendingMilestones()
{
    TArray<FDir_Milestone> Pending;
    for (const FDir_Milestone& Milestone : Milestones)
    {
        if (!Milestone.bIsComplete)
        {
            Pending.Add(Milestone);
        }
    }
    return Pending;
}

FDir_ProductionMetrics UDir_ProductionPipeline::GetProductionMetrics()
{
    return Metrics;
}

void UDir_ProductionPipeline::UpdateBudgetTracking(float CostDelta)
{
    Metrics.BudgetUsed += CostDelta;
    Metrics.BudgetRemaining = FMath::Max(0.0f, 75.0f - Metrics.BudgetUsed); // $75 daily budget
}

bool UDir_ProductionPipeline::ValidateMinimumViablePrototype()
{
    return bCharacterMovementComplete && bTerrainSystemComplete;
}

bool UDir_ProductionPipeline::ValidateCharacterMovement()
{
    // Check if TranspersonalCharacter exists and has movement
    UWorld* World = GetWorld();
    if (!World) return false;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor->GetName().Contains(TEXT("Character")))
        {
            return true; // Found character actor
        }
    }
    return false;
}

bool UDir_ProductionPipeline::ValidateDinosaurAI()
{
    // Check if dinosaur actors exist with AI controllers
    UWorld* World = GetWorld();
    if (!World) return false;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor->GetName().Contains(TEXT("Dinosaur")) || Actor->GetName().Contains(TEXT("TRex")))
        {
            return true; // Found dinosaur actor
        }
    }
    return false;
}

bool UDir_ProductionPipeline::ValidateTerrainSystem()
{
    // Check if landscape exists
    UWorld* World = GetWorld();
    if (!World) return false;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor->GetName().Contains(TEXT("Landscape")))
        {
            return true; // Found landscape
        }
    }
    return false;
}

void UDir_ProductionPipeline::UpdateProductionMetrics()
{
    Metrics.TotalCycles = CurrentCycleNumber;
    
    // Count active agents
    int32 ActiveCount = 0;
    for (const auto& Status : AgentStatuses)
    {
        if (Status.Value == EDir_AgentStatus::Working)
        {
            ActiveCount++;
        }
    }
    Metrics.ActiveAgents = ActiveCount;
    
    // Calculate overall progress
    int32 CompletedMilestones = 0;
    for (const FDir_Milestone& Milestone : Milestones)
    {
        if (Milestone.bIsComplete)
        {
            CompletedMilestones++;
        }
    }
    
    if (Milestones.Num() > 0)
    {
        Metrics.OverallProgress = (float)CompletedMilestones / Milestones.Num() * 100.0f;
    }
}

bool UDir_ProductionPipeline::CheckMilestoneDependencies(const FDir_Milestone& Milestone)
{
    for (int32 RequiredAgent : Milestone.RequiredAgents)
    {
        if (AgentStatuses.Contains(RequiredAgent))
        {
            EDir_AgentStatus Status = AgentStatuses[RequiredAgent];
            if (Status != EDir_AgentStatus::Complete && Status != EDir_AgentStatus::Working)
            {
                return false;
            }
        }
    }
    return true;
}

void UDir_ProductionPipeline::TriggerEmergencyProtocol(const FString& Reason)
{
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY PROTOCOL TRIGGERED: %s"), *Reason);
    
    // Reset all agents to idle except critical path
    for (auto& Status : AgentStatuses)
    {
        if (Status.Key <= 3) // Keep Director, Architect, Core Systems active
        {
            continue;
        }
        Status.Value = EDir_AgentStatus::Idle;
    }
    
    // Add to critical blocks
    Metrics.CriticalBlocks.Add(FString::Printf(TEXT("Emergency at cycle %d: %s"), CurrentCycleNumber, *Reason));
}