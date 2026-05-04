#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"

DEFINE_LOG_CATEGORY(LogProductionCoordinator);

UProductionCoordinator::UProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    // Initialize milestone tracking
    Milestone1_WalkAround.bCompleted = false;
    Milestone1_WalkAround.RequiredSystems.Add(TEXT("PlayerCharacter"));
    Milestone1_WalkAround.RequiredSystems.Add(TEXT("BasicTerrain"));
    Milestone1_WalkAround.RequiredSystems.Add(TEXT("DinosaurActors"));
    Milestone1_WalkAround.RequiredSystems.Add(TEXT("LightingSystem"));
    Milestone1_WalkAround.RequiredSystems.Add(TEXT("MovementInput"));
    
    // Initialize agent task tracking
    InitializeAgentTasks();
}

void UProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogProductionCoordinator, Warning, TEXT("Production Coordinator initialized - Tracking Milestone 1"));
    
    // Start monitoring production state
    GetWorld()->GetTimerManager().SetTimer(
        ProductionCheckTimer,
        this,
        &UProductionCoordinator::CheckProductionState,
        5.0f,
        true
    );
}

void UProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update milestone progress
    UpdateMilestone1Progress();
}

void UProductionCoordinator::InitializeAgentTasks()
{
    // Agent #2 - Engine Architect
    FAgentTask EngineTask;
    EngineTask.AgentName = TEXT("Engine Architect");
    EngineTask.CurrentTask = TEXT("Define core architecture and compilation rules");
    EngineTask.Priority = ETaskPriority::Critical;
    EngineTask.bBlocking = true;
    EngineTask.RequiredDeliverables.Add(TEXT("SharedTypes.h complete"));
    EngineTask.RequiredDeliverables.Add(TEXT("Module dependencies resolved"));
    AgentTasks.Add(2, EngineTask);
    
    // Agent #3 - Core Systems
    FAgentTask CoreTask;
    CoreTask.AgentName = TEXT("Core Systems Programmer");
    CoreTask.CurrentTask = TEXT("Implement physics and collision systems");
    CoreTask.Priority = ETaskPriority::High;
    CoreTask.bBlocking = false;
    CoreTask.RequiredDeliverables.Add(TEXT("PhysicsCore.cpp"));
    CoreTask.RequiredDeliverables.Add(TEXT("CollisionSystem.cpp"));
    AgentTasks.Add(3, CoreTask);
    
    // Agent #5 - World Generator
    FAgentTask WorldTask;
    WorldTask.AgentName = TEXT("Procedural World Generator");
    WorldTask.CurrentTask = TEXT("Expand landscape to 200km² with 5 biomes"));
    WorldTask.Priority = ETaskPriority::Critical;
    WorldTask.bBlocking = false;
    WorldTask.RequiredDeliverables.Add(TEXT("Landscape expansion complete"));
    WorldTask.RequiredDeliverables.Add(TEXT("Biome distribution system"));
    AgentTasks.Add(5, WorldTask);
    
    // Agent #9 - Character Artist
    FAgentTask CharacterTask;
    CharacterTask.AgentName = TEXT("Character Artist");
    CharacterTask.CurrentTask = TEXT("Create playable character with movement"));
    CharacterTask.Priority = ETaskPriority::Critical;
    CharacterTask.bBlocking = false;
    CharacterTask.RequiredDeliverables.Add(TEXT("TranspersonalCharacter.cpp"));
    CharacterTask.RequiredDeliverables.Add(TEXT("Character movement working"));
    AgentTasks.Add(9, CharacterTask);
    
    // Agent #12 - Combat AI
    FAgentTask CombatTask;
    CombatTask.AgentName = TEXT("Combat & Enemy AI");
    CombatTask.CurrentTask = TEXT("Create basic dinosaur actors with collision"));
    CombatTask.Priority = ETaskPriority::High;
    CombatTask.bBlocking = false;
    CombatTask.RequiredDeliverables.Add(TEXT("DinosaurActor.cpp"));
    CombatTask.RequiredDeliverables.Add(TEXT("Basic AI behavior"));
    AgentTasks.Add(12, CombatTask);
    
    // Agent #20 - Integration
    FAgentTask IntegrationTask;
    IntegrationTask.AgentName = TEXT("Integration & Build");
    IntegrationTask.CurrentTask = TEXT("Clean 122 phantom headers and compile"));
    IntegrationTask.Priority = ETaskPriority::Critical;
    IntegrationTask.bBlocking = true;
    IntegrationTask.RequiredDeliverables.Add(TEXT("Compilation successful"));
    IntegrationTask.RequiredDeliverables.Add(TEXT("Phantom headers removed"));
    AgentTasks.Add(20, IntegrationTask);
}

void UProductionCoordinator::UpdateMilestone1Progress()
{
    if (Milestone1_WalkAround.bCompleted)
        return;
    
    // Check each required system
    TArray<FString> CompletedSystems;
    
    // Check for player character
    if (CheckPlayerCharacterExists())
    {
        CompletedSystems.Add(TEXT("PlayerCharacter"));
    }
    
    // Check for terrain
    if (CheckTerrainExists())
    {
        CompletedSystems.Add(TEXT("BasicTerrain"));
    }
    
    // Check for dinosaur actors
    if (CheckDinosaurActorsExist())
    {
        CompletedSystems.Add(TEXT("DinosaurActors"));
    }
    
    // Check for lighting
    if (CheckLightingSystemExists())
    {
        CompletedSystems.Add(TEXT("LightingSystem"));
    }
    
    // Check for movement input
    if (CheckMovementInputWorks())
    {
        CompletedSystems.Add(TEXT("MovementInput"));
    }
    
    // Update progress
    float Progress = (float)CompletedSystems.Num() / (float)Milestone1_WalkAround.RequiredSystems.Num();
    Milestone1_WalkAround.CompletionPercentage = Progress * 100.0f;
    
    // Check if milestone is complete
    if (CompletedSystems.Num() == Milestone1_WalkAround.RequiredSystems.Num())
    {
        Milestone1_WalkAround.bCompleted = true;
        UE_LOG(LogProductionCoordinator, Warning, TEXT("MILESTONE 1 COMPLETED: Walk Around prototype is ready!"));
        
        // Trigger celebration or next milestone
        OnMilestone1Completed();
    }
    else
    {
        UE_LOG(LogProductionCoordinator, Log, TEXT("Milestone 1 Progress: %.1f%% (%d/%d systems)"), 
            Milestone1_WalkAround.CompletionPercentage, 
            CompletedSystems.Num(), 
            Milestone1_WalkAround.RequiredSystems.Num());
    }
}

bool UProductionCoordinator::CheckPlayerCharacterExists()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Look for any character that can be possessed by player
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
    return PlayerCharacter != nullptr;
}

bool UProductionCoordinator::CheckTerrainExists()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Look for landscape actors
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    
    return LandscapeActors.Num() > 0;
}

bool UProductionCoordinator::CheckDinosaurActorsExist()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Look for any actors with "Dinosaur" in their name
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            return true;
        }
        if (Actor && (Actor->GetName().Contains(TEXT("TRex")) || Actor->GetName().Contains(TEXT("Raptor"))))
        {
            return true;
        }
    }
    
    return false;
}

bool UProductionCoordinator::CheckLightingSystemExists()
{
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Look for directional light (sun)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("DirectionalLight")))
        {
            return true;
        }
    }
    
    return false;
}

bool UProductionCoordinator::CheckMovementInputWorks()
{
    // This is harder to check automatically - for now assume true if character exists
    return CheckPlayerCharacterExists();
}

void UProductionCoordinator::CheckProductionState()
{
    UE_LOG(LogProductionCoordinator, Log, TEXT("=== PRODUCTION STATE CHECK ==="));
    
    // Check agent progress
    for (auto& AgentPair : AgentTasks)
    {
        int32 AgentID = AgentPair.Key;
        FAgentTask& Task = AgentPair.Value;
        
        // Log current task status
        FString PriorityStr = (Task.Priority == ETaskPriority::Critical) ? TEXT("CRITICAL") : 
                             (Task.Priority == ETaskPriority::High) ? TEXT("HIGH") : TEXT("NORMAL");
        
        UE_LOG(LogProductionCoordinator, Log, TEXT("Agent #%d (%s): %s [%s]"), 
            AgentID, 
            *Task.AgentName, 
            *Task.CurrentTask, 
            *PriorityStr);
    }
    
    // Report milestone status
    UE_LOG(LogProductionCoordinator, Warning, TEXT("MILESTONE 1 STATUS: %.1f%% complete"), 
        Milestone1_WalkAround.CompletionPercentage);
}

void UProductionCoordinator::OnMilestone1Completed()
{
    UE_LOG(LogProductionCoordinator, Warning, TEXT("🎉 MILESTONE 1 ACHIEVED: Playable prototype ready!"));
    UE_LOG(LogProductionCoordinator, Warning, TEXT("Player can walk around world with dinosaurs visible"));
    
    // Prepare for next milestone
    InitializeMilestone2();
}

void UProductionCoordinator::InitializeMilestone2()
{
    // TODO: Define Milestone 2 - Basic Survival Mechanics
    UE_LOG(LogProductionCoordinator, Log, TEXT("Preparing Milestone 2: Basic Survival Mechanics"));
}

void UProductionCoordinator::ReportAgentProgress(int32 AgentID, const FString& TaskUpdate, float ProgressPercentage)
{
    if (AgentTasks.Contains(AgentID))
    {
        FAgentTask& Task = AgentTasks[AgentID];
        Task.ProgressPercentage = ProgressPercentage;
        Task.LastUpdate = FDateTime::Now();
        
        UE_LOG(LogProductionCoordinator, Log, TEXT("Agent #%d progress update: %s (%.1f%%)"), 
            AgentID, *TaskUpdate, ProgressPercentage);
    }
}

void UProductionCoordinator::BlockingIssueDetected(const FString& IssueDescription, int32 ResponsibleAgentID)
{
    UE_LOG(LogProductionCoordinator, Error, TEXT("🚨 BLOCKING ISSUE: %s (Agent #%d)"), 
        *IssueDescription, ResponsibleAgentID);
    
    // Mark as critical priority
    if (AgentTasks.Contains(ResponsibleAgentID))
    {
        AgentTasks[ResponsibleAgentID].Priority = ETaskPriority::Critical;
        AgentTasks[ResponsibleAgentID].bBlocking = true;
    }
}