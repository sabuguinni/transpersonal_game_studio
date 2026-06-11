#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AProductionCoordinator::AProductionCoordinator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize production state
    CurrentCycle = 20;
    ProductionPhase = TEXT("MILESTONE_1_WALKABOUT");
    
    // Initialize milestone status
    CurrentMilestone = FDir_MilestoneStatus();
    CurrentMilestone.MilestoneName = TEXT("Milestone 1 - Walk Around");
}

void AProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize agent tasks and assess current state
    InitializeAgentTasks();
    UpdateMilestoneStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Cycle %d initialized - %s"), CurrentCycle, *ProductionPhase);
}

void AProductionCoordinator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update milestone status every few seconds
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    if (UpdateTimer >= 5.0f) // Update every 5 seconds
    {
        UpdateMilestoneStatus();
        UpdateTimer = 0.0f;
    }
}

void AProductionCoordinator::InitializeAgentTasks()
{
    CreateAgentTaskList();
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: %d agent tasks initialized"), AgentTasks.Num());
}

void AProductionCoordinator::CreateAgentTaskList()
{
    AgentTasks.Empty();
    
    // Priority agents for Milestone 1
    FDir_AgentTask Task;
    
    // Agent #2 - Engine Architect
    Task = FDir_AgentTask();
    Task.AgentNumber = 2;
    Task.AgentName = TEXT("Engine Architect");
    Task.TaskDescription = TEXT("Fix compilation errors and establish core architecture");
    Task.DeliverableType = TEXT("CODE");
    Task.Priority = 2.0f; // High priority
    AgentTasks.Add(Task);
    
    // Agent #3 - Core Systems
    Task = FDir_AgentTask();
    Task.AgentNumber = 3;
    Task.AgentName = TEXT("Core Systems Programmer");
    Task.TaskDescription = TEXT("Implement physics and collision systems");
    Task.DeliverableType = TEXT("CODE");
    Task.Priority = 2.0f;
    AgentTasks.Add(Task);
    
    // Agent #5 - World Generator
    Task = FDir_AgentTask();
    Task.AgentNumber = 5;
    Task.AgentName = TEXT("Procedural World Generator");
    Task.TaskDescription = TEXT("Create varied terrain with hills and valleys");
    Task.DeliverableType = TEXT("UE5_COMMAND");
    Task.Priority = 2.0f;
    AgentTasks.Add(Task);
    
    // Agent #9 - Character Artist
    Task = FDir_AgentTask();
    Task.AgentNumber = 9;
    Task.AgentName = TEXT("Character Artist Agent");
    Task.TaskDescription = TEXT("Improve character movement and appearance");
    Task.DeliverableType = TEXT("BLUEPRINT");
    Task.Priority = 2.0f;
    AgentTasks.Add(Task);
    
    // Agent #10 - Animation
    Task = FDir_AgentTask();
    Task.AgentNumber = 10;
    Task.AgentName = TEXT("Animation Agent");
    Task.TaskDescription = TEXT("Add basic walk/run/jump animations");
    Task.DeliverableType = TEXT("BLUEPRINT");
    Task.Priority = 2.0f;
    AgentTasks.Add(Task);
    
    // Agent #12 - Combat & Enemy AI
    Task = FDir_AgentTask();
    Task.AgentNumber = 12;
    Task.AgentName = TEXT("Combat & Enemy AI Agent");
    Task.TaskDescription = TEXT("Create survival HUD and dinosaur basic AI");
    Task.DeliverableType = TEXT("CODE");
    Task.Priority = 2.0f;
    AgentTasks.Add(Task);
    
    // Add more agents with lower priority
    for (int32 i = 4; i <= 19; i++)
    {
        if (i == 5 || i == 9 || i == 10 || i == 12) continue; // Already added above
        
        Task = FDir_AgentTask();
        Task.AgentNumber = i;
        Task.AgentName = FString::Printf(TEXT("Agent #%d"), i);
        Task.TaskDescription = TEXT("Support Milestone 1 objectives");
        Task.DeliverableType = TEXT("CODE");
        Task.Priority = 1.0f; // Normal priority
        AgentTasks.Add(Task);
    }
}

void AProductionCoordinator::UpdateMilestoneStatus()
{
    AssessLevelState();
    
    // Calculate completion percentage
    int32 CompletedRequirements = 0;
    int32 TotalRequirements = 5;
    
    if (CurrentMilestone.bPlayableCharacter) CompletedRequirements++;
    if (CurrentMilestone.bTerrainExists) CompletedRequirements++;
    if (CurrentMilestone.bDinosaursPlaced) CompletedRequirements++;
    if (CurrentMilestone.bLightingSetup) CompletedRequirements++;
    if (CurrentMilestone.bSurvivalHUD) CompletedRequirements++;
    
    CurrentMilestone.CompletionPercentage = (float)CompletedRequirements / (float)TotalRequirements * 100.0f;
}

void AProductionCoordinator::AssessLevelState()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Check for playable character
    CurrentMilestone.bPlayableCharacter = CountActorsByType(TEXT("Character")) > 0;
    
    // Check for terrain
    CurrentMilestone.bTerrainExists = CountActorsByType(TEXT("Landscape")) > 0;
    
    // Check for dinosaurs
    CurrentMilestone.bDinosaursPlaced = CountActorsByType(TEXT("Dinosaur")) > 0 || 
                                       CountActorsByType(TEXT("TRex")) > 0 || 
                                       CountActorsByType(TEXT("Raptor")) > 0;
    
    // Check for lighting
    CurrentMilestone.bLightingSetup = CountActorsByType(TEXT("Light")) > 0 || 
                                     CountActorsByType(TEXT("Sun")) > 0;
    
    // Check for survival HUD (placeholder - will be implemented by Agent #12)
    CurrentMilestone.bSurvivalHUD = false; // TODO: Implement HUD detection
}

int32 AProductionCoordinator::CountActorsByType(const FString& ActorType) const
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(ActorType))
        {
            Count++;
        }
    }
    
    return Count;
}

float AProductionCoordinator::GetOverallProgress() const
{
    return CurrentMilestone.CompletionPercentage;
}

void AProductionCoordinator::MarkTaskCompleted(int32 AgentNumber, const FString& DeliverableType)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentNumber == AgentNumber && Task.DeliverableType == DeliverableType)
        {
            Task.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Task completed - Agent #%d (%s)"), AgentNumber, *DeliverableType);
            break;
        }
    }
}

TArray<FDir_AgentTask> AProductionCoordinator::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (!Task.bIsCompleted)
        {
            PendingTasks.Add(Task);
        }
    }
    
    return PendingTasks;
}

bool AProductionCoordinator::IsMilestoneComplete() const
{
    return CurrentMilestone.CompletionPercentage >= 100.0f;
}

void AProductionCoordinator::DebugPrintProductionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION STATUS DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Cycle: %d"), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %s"), *ProductionPhase);
    UE_LOG(LogTemp, Warning, TEXT("Milestone: %s (%.1f%% complete)"), *CurrentMilestone.MilestoneName, CurrentMilestone.CompletionPercentage);
    
    UE_LOG(LogTemp, Warning, TEXT("Requirements:"));
    UE_LOG(LogTemp, Warning, TEXT("  Playable Character: %s"), CurrentMilestone.bPlayableCharacter ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("  Terrain Exists: %s"), CurrentMilestone.bTerrainExists ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("  Dinosaurs Placed: %s"), CurrentMilestone.bDinosaursPlaced ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("  Lighting Setup: %s"), CurrentMilestone.bLightingSetup ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("  Survival HUD: %s"), CurrentMilestone.bSurvivalHUD ? TEXT("YES") : TEXT("NO"));
    
    TArray<FDir_AgentTask> PendingTasks = GetPendingTasks();
    UE_LOG(LogTemp, Warning, TEXT("Pending Tasks: %d"), PendingTasks.Num());
    
    for (const FDir_AgentTask& Task : PendingTasks)
    {
        if (Task.Priority >= 2.0f) // High priority tasks only
        {
            UE_LOG(LogTemp, Warning, TEXT("  PRIORITY: Agent #%d - %s"), Task.AgentNumber, *Task.TaskDescription);
        }
    }
}

void AProductionCoordinator::ResetProductionState()
{
    CurrentMilestone = FDir_MilestoneStatus();
    AgentTasks.Empty();
    InitializeAgentTasks();
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Production state reset"));
}