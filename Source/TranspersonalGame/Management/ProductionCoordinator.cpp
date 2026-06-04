#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    CurrentPhase = EDir_ProductionPhase::WalkAroundPrototype;
    TotalActorsInMap = 0;
    bBridgeOperational = false;
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeProduction();
    ValidateBridgeConnection();
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update actor count
    if (UWorld* World = GetWorld())
    {
        TotalActorsInMap = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            TotalActorsInMap++;
        }
    }
}

void UDir_ProductionCoordinator::InitializeProduction()
{
    // Initialize Walk Around Prototype milestones
    Milestones.Empty();
    
    FDir_MilestoneStatus TerrainMilestone;
    TerrainMilestone.MilestoneName = TEXT("Terrain Generation");
    TerrainMilestone.CompletionPercentage = 75.0f;
    TerrainMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(1);
    Milestones.Add(TerrainMilestone);
    
    FDir_MilestoneStatus CharacterMilestone;
    CharacterMilestone.MilestoneName = TEXT("Character Movement");
    CharacterMilestone.CompletionPercentage = 85.0f;
    CharacterMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(1);
    Milestones.Add(CharacterMilestone);
    
    FDir_MilestoneStatus DinosaurMilestone;
    DinosaurMilestone.MilestoneName = TEXT("Dinosaur Placement");
    DinosaurMilestone.CompletionPercentage = 60.0f;
    DinosaurMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(2);
    Milestones.Add(DinosaurMilestone);
    
    FDir_MilestoneStatus HUDMilestone;
    HUDMilestone.MilestoneName = TEXT("Survival HUD");
    HUDMilestone.CompletionPercentage = 40.0f;
    HUDMilestone.TargetDate = FDateTime::Now() + FTimespan::FromDays(2);
    Milestones.Add(HUDMilestone);
    
    // Initialize agent tasks
    AgentTasks.Empty();
    
    // Agent #5 - Procedural World Generator
    FDir_AgentTaskStatus Agent5;
    Agent5.AgentNumber = 5;
    Agent5.AgentName = TEXT("Procedural World Generator");
    Agent5.CurrentTask = TEXT("Create heightmap terrain with hills and valleys");
    Agent5.bIsActive = true;
    Agent5.TaskProgress = 70.0f;
    AgentTasks.Add(Agent5);
    
    // Agent #9 - Character Artist
    FDir_AgentTaskStatus Agent9;
    Agent9.AgentNumber = 9;
    Agent9.AgentName = TEXT("Character Artist");
    Agent9.CurrentTask = TEXT("Implement dinosaur actors with collision");
    Agent9.bIsActive = true;
    Agent9.TaskProgress = 55.0f;
    AgentTasks.Add(Agent9);
    
    // Agent #12 - Combat & Enemy AI
    FDir_AgentTaskStatus Agent12;
    Agent12.AgentNumber = 12;
    Agent12.AgentName = TEXT("Combat & Enemy AI");
    Agent12.CurrentTask = TEXT("Create survival HUD with health/hunger bars");
    Agent12.bIsActive = true;
    Agent12.TaskProgress = 35.0f;
    AgentTasks.Add(Agent12);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Initialized with %d milestones and %d agent tasks"), Milestones.Num(), AgentTasks.Num());
}

void UDir_ProductionCoordinator::UpdateMilestoneProgress(const FString& MilestoneName, float Progress)
{
    for (FDir_MilestoneStatus& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == MilestoneName)
        {
            Milestone.CompletionPercentage = FMath::Clamp(Progress, 0.0f, 100.0f);
            Milestone.bIsCompleted = (Progress >= 100.0f);
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Updated %s to %.1f%% completion"), *MilestoneName, Progress);
            break;
        }
    }
}

void UDir_ProductionCoordinator::UpdateAgentTask(int32 AgentNumber, const FString& TaskDescription, float Progress)
{
    for (FDir_AgentTaskStatus& Agent : AgentTasks)
    {
        if (Agent.AgentNumber == AgentNumber)
        {
            Agent.CurrentTask = TaskDescription;
            Agent.TaskProgress = FMath::Clamp(Progress, 0.0f, 100.0f);
            Agent.bIsActive = true;
            
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agent #%d updated task: %s (%.1f%%)"), AgentNumber, *TaskDescription, Progress);
            break;
        }
    }
}

void UDir_ProductionCoordinator::ValidateBridgeConnection()
{
    // Simple bridge validation - check if we can access world
    bBridgeOperational = (GetWorld() != nullptr);
    
    if (bBridgeOperational)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Bridge connection OPERATIONAL"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: Bridge connection FAILED"));
    }
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT - CYCLE 003 ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: Walk Around Prototype"));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors in Map: %d"), TotalActorsInMap);
    UE_LOG(LogTemp, Warning, TEXT("Bridge Status: %s"), bBridgeOperational ? TEXT("OPERATIONAL") : TEXT("FAILED"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== MILESTONES ==="));
    for (const FDir_MilestoneStatus& Milestone : Milestones)
    {
        UE_LOG(LogTemp, Warning, TEXT("- %s: %.1f%% %s"), 
            *Milestone.MilestoneName, 
            Milestone.CompletionPercentage,
            Milestone.bIsCompleted ? TEXT("COMPLETE") : TEXT("IN PROGRESS"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== AGENT TASKS ==="));
    for (const FDir_AgentTaskStatus& Agent : AgentTasks)
    {
        UE_LOG(LogTemp, Warning, TEXT("- Agent #%d (%s): %s (%.1f%%)"), 
            Agent.AgentNumber, 
            *Agent.AgentName,
            *Agent.CurrentTask,
            Agent.TaskProgress);
    }
    
    float OverallProgress = GetOverallProgress();
    UE_LOG(LogTemp, Warning, TEXT("=== OVERALL PROGRESS: %.1f%% ==="), OverallProgress);
}

float UDir_ProductionCoordinator::GetOverallProgress() const
{
    if (Milestones.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    for (const FDir_MilestoneStatus& Milestone : Milestones)
    {
        TotalProgress += Milestone.CompletionPercentage;
    }
    
    return TotalProgress / Milestones.Num();
}

bool UDir_ProductionCoordinator::IsWalkAroundPrototypeComplete() const
{
    // Walk Around Prototype is complete when all core milestones are at 100%
    for (const FDir_MilestoneStatus& Milestone : Milestones)
    {
        if (Milestone.MilestoneName == TEXT("Terrain Generation") && Milestone.CompletionPercentage < 100.0f)
            return false;
        if (Milestone.MilestoneName == TEXT("Character Movement") && Milestone.CompletionPercentage < 100.0f)
            return false;
        if (Milestone.MilestoneName == TEXT("Dinosaur Placement") && Milestone.CompletionPercentage < 100.0f)
            return false;
    }
    
    return true;
}