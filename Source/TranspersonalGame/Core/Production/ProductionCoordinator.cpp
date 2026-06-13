#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Level.h"
#include "Kismet/GameplayStatics.h"

UDir_ProductionCoordinator::UDir_ProductionCoordinator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Update every 5 seconds
    
    CurrentPhase = EDir_ProductionPhase::CoreSystems;
    CurrentCycle = 20;
    CycleObjective = TEXT("Create Minimum Viable Playable Prototype");
    
    // Initialize metrics
    CurrentMetrics = FDir_ProductionMetrics();
}

void UDir_ProductionCoordinator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Coordinator initialized for Cycle %d"), CurrentCycle);
    InitializeCycle020Tasks();
    UpdateProductionMetrics();
}

void UDir_ProductionCoordinator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateProductionMetrics();
    CheckTaskDependencies();
    
    // Log critical issues
    if (CurrentMetrics.CompilationErrors > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: %d compilation errors detected"), CurrentMetrics.CompilationErrors);
    }
    
    if (CurrentMetrics.ActiveActors > 8000)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Actor count %d exceeds limit of 8000"), CurrentMetrics.ActiveActors);
    }
}

void UDir_ProductionCoordinator::InitializeCycle020Tasks()
{
    AgentTasks.Empty();
    
    // Agent #02 - Engine Architect
    FDir_AgentTask EngineTask;
    EngineTask.AgentName = TEXT("Agent_02_Engine_Architect");
    EngineTask.TaskDescription = TEXT("UE5.5 compatibility validation and core architecture review");
    EngineTask.Status = EDir_AgentStatus::Pending;
    EngineTask.Priority = 10;
    EngineTask.ExpectedDeliverables.Add(TEXT("EngineCompatibility.cpp"));
    EngineTask.ExpectedDeliverables.Add(TEXT("CoreArchitecture.cpp"));
    EngineTask.EstimatedHours = 2.0f;
    AgentTasks.Add(EngineTask);
    
    // Agent #03 - Core Systems
    FDir_AgentTask CoreTask;
    CoreTask.AgentName = TEXT("Agent_03_Core_Systems");
    CoreTask.TaskDescription = TEXT("Physics and collision for character-dinosaur interaction");
    CoreTask.Status = EDir_AgentStatus::Pending;
    CoreTask.Priority = 9;
    CoreTask.Dependencies.Add(TEXT("Agent_02_Engine_Architect"));
    CoreTask.ExpectedDeliverables.Add(TEXT("PhysicsInteractionSystem.cpp"));
    CoreTask.ExpectedDeliverables.Add(TEXT("CollisionManager.cpp"));
    CoreTask.EstimatedHours = 3.0f;
    AgentTasks.Add(CoreTask);
    
    // Agent #05 - World Generator
    FDir_AgentTask WorldTask;
    WorldTask.AgentName = TEXT("Agent_05_World_Generator");
    WorldTask.TaskDescription = TEXT("Create real terrain with height variation in MinPlayableMap");
    WorldTask.Status = EDir_AgentStatus::Pending;
    WorldTask.Priority = 8;
    WorldTask.ExpectedDeliverables.Add(TEXT("TerrainGenerator.cpp"));
    WorldTask.ExpectedDeliverables.Add(TEXT("BiomeSystem.cpp"));
    WorldTask.EstimatedHours = 2.5f;
    AgentTasks.Add(WorldTask);
    
    // Agent #09 - Character Artist
    FDir_AgentTask CharacterTask;
    CharacterTask.AgentName = TEXT("Agent_09_Character_Artist");
    CharacterTask.TaskDescription = TEXT("Setup MetaHuman character with survival stats HUD");
    CharacterTask.Status = EDir_AgentStatus::Pending;
    CharacterTask.Priority = 7;
    CharacterTask.Dependencies.Add(TEXT("Agent_03_Core_Systems"));
    CharacterTask.ExpectedDeliverables.Add(TEXT("SurvivalCharacter.cpp"));
    CharacterTask.ExpectedDeliverables.Add(TEXT("SurvivalHUD.cpp"));
    CharacterTask.EstimatedHours = 3.5f;
    AgentTasks.Add(CharacterTask);
    
    // Agent #10 - Animation
    FDir_AgentTask AnimationTask;
    AnimationTask.AgentName = TEXT("Agent_10_Animation");
    AnimationTask.TaskDescription = TEXT("Character movement and dinosaur basic animations");
    AnimationTask.Status = EDir_AgentStatus::Pending;
    AnimationTask.Priority = 6;
    AnimationTask.Dependencies.Add(TEXT("Agent_09_Character_Artist"));
    AnimationTask.ExpectedDeliverables.Add(TEXT("CharacterAnimController.cpp"));
    AnimationTask.ExpectedDeliverables.Add(TEXT("DinosaurAnimBP.cpp"));
    AnimationTask.EstimatedHours = 4.0f;
    AgentTasks.Add(AnimationTask);
    
    // Agent #12 - Combat AI
    FDir_AgentTask CombatTask;
    CombatTask.AgentName = TEXT("Agent_12_Combat_AI");
    CombatTask.TaskDescription = TEXT("Basic dinosaur AI behavior and territory system");
    CombatTask.Status = EDir_AgentStatus::Pending;
    CombatTask.Priority = 5;
    CombatTask.Dependencies.Add(TEXT("Agent_10_Animation"));
    CombatTask.ExpectedDeliverables.Add(TEXT("DinosaurAI.cpp"));
    CombatTask.ExpectedDeliverables.Add(TEXT("TerritorySystem.cpp"));
    CombatTask.EstimatedHours = 3.0f;
    AgentTasks.Add(CombatTask);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d tasks for Cycle 020"), AgentTasks.Num());
}

void UDir_ProductionCoordinator::UpdateAgentTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus)
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.AgentName == AgentName)
        {
            Task.Status = NewStatus;
            UE_LOG(LogTemp, Log, TEXT("Agent %s status updated to %d"), *AgentName, (int32)NewStatus);
            break;
        }
    }
    
    LogProductionState();
}

void UDir_ProductionCoordinator::ValidatePlayablePrototype()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    bool bHasCharacter = false;
    bool bHasTerrain = false;
    bool bHasDinosaurs = false;
    bool bHasLighting = false;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        FString ActorName = Actor->GetClass()->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("transpersonal")))
        {
            bHasCharacter = true;
        }
        else if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")))
        {
            bHasTerrain = true;
        }
        else if (ActorLabel.Contains(TEXT("trex")) || ActorLabel.Contains(TEXT("raptor")) || 
                 ActorLabel.Contains(TEXT("brachi")) || ActorLabel.Contains(TEXT("tricera")))
        {
            bHasDinosaurs = true;
        }
        else if (ActorName.Contains(TEXT("light")))
        {
            bHasLighting = true;
        }
    }
    
    CurrentMetrics.bPlayablePrototypeReady = bHasCharacter && bHasTerrain && bHasDinosaurs && bHasLighting;
    
    UE_LOG(LogTemp, Warning, TEXT("Prototype Validation - Character: %s, Terrain: %s, Dinosaurs: %s, Lighting: %s"),
           bHasCharacter ? TEXT("OK") : TEXT("MISSING"),
           bHasTerrain ? TEXT("OK") : TEXT("MISSING"),
           bHasDinosaurs ? TEXT("OK") : TEXT("MISSING"),
           bHasLighting ? TEXT("OK") : TEXT("MISSING"));
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetPendingTasks() const
{
    TArray<FDir_AgentTask> PendingTasks;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Pending)
        {
            PendingTasks.Add(Task);
        }
    }
    return PendingTasks;
}

TArray<FDir_AgentTask> UDir_ProductionCoordinator::GetBlockedTasks() const
{
    TArray<FDir_AgentTask> BlockedTasks;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Blocked)
        {
            BlockedTasks.Add(Task);
        }
    }
    return BlockedTasks;
}

bool UDir_ProductionCoordinator::IsPlayablePrototypeReady() const
{
    return CurrentMetrics.bPlayablePrototypeReady;
}

void UDir_ProductionCoordinator::GenerateProductionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PRODUCTION REPORT CYCLE %d ==="), CurrentCycle);
    UE_LOG(LogTemp, Warning, TEXT("Objective: %s"), *CycleObjective);
    UE_LOG(LogTemp, Warning, TEXT("Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Total Tasks: %d"), AgentTasks.Num());
    
    int32 CompletedTasks = 0;
    int32 PendingTasks = 0;
    int32 BlockedTasks = 0;
    
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        switch (Task.Status)
        {
            case EDir_AgentStatus::Completed:
                CompletedTasks++;
                break;
            case EDir_AgentStatus::Pending:
                PendingTasks++;
                break;
            case EDir_AgentStatus::Blocked:
                BlockedTasks++;
                break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Completed: %d, Pending: %d, Blocked: %d"), 
           CompletedTasks, PendingTasks, BlockedTasks);
    UE_LOG(LogTemp, Warning, TEXT("Prototype Ready: %s"), 
           CurrentMetrics.bPlayablePrototypeReady ? TEXT("YES") : TEXT("NO"));
}

void UDir_ProductionCoordinator::UpdateProductionMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    CurrentMetrics.ActiveActors = AllActors.Num();
    
    // Calculate progress based on completed tasks
    int32 CompletedTasks = 0;
    for (const FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Completed)
        {
            CompletedTasks++;
        }
    }
    
    CurrentMetrics.OverallProgress = AgentTasks.Num() > 0 ? 
        (float)CompletedTasks / (float)AgentTasks.Num() * 100.0f : 0.0f;
}

void UDir_ProductionCoordinator::CheckTaskDependencies()
{
    for (FDir_AgentTask& Task : AgentTasks)
    {
        if (Task.Status == EDir_AgentStatus::Pending && Task.Dependencies.Num() > 0)
        {
            bool bAllDependenciesCompleted = true;
            for (const FString& Dependency : Task.Dependencies)
            {
                bool bDependencyCompleted = false;
                for (const FDir_AgentTask& OtherTask : AgentTasks)
                {
                    if (OtherTask.AgentName == Dependency && 
                        OtherTask.Status == EDir_AgentStatus::Completed)
                    {
                        bDependencyCompleted = true;
                        break;
                    }
                }
                if (!bDependencyCompleted)
                {
                    bAllDependenciesCompleted = false;
                    break;
                }
            }
            
            if (!bAllDependenciesCompleted)
            {
                Task.Status = EDir_AgentStatus::Blocked;
            }
        }
    }
}

void UDir_ProductionCoordinator::LogProductionState()
{
    UE_LOG(LogTemp, Log, TEXT("Production State - Cycle %d Progress: %.1f%%"), 
           CurrentCycle, CurrentMetrics.OverallProgress);
}

// Production Director Actor Implementation
ADir_ProductionDirector::ADir_ProductionDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    
    ProductionCoordinator = CreateDefaultSubobject<UDir_ProductionCoordinator>(TEXT("ProductionCoordinator"));
    
    bAutoManageProduction = true;
    ProductionUpdateInterval = 10.0f;
    LastUpdateTime = 0.0f;
}

void ADir_ProductionDirector::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Production Director started for Cycle 020"));
    StartCycle020();
}

void ADir_ProductionDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoManageProduction)
    {
        LastUpdateTime += DeltaTime;
        if (LastUpdateTime >= ProductionUpdateInterval)
        {
            LastUpdateTime = 0.0f;
            if (ProductionCoordinator)
            {
                ProductionCoordinator->ValidatePlayablePrototype();
            }
        }
    }
}

void ADir_ProductionDirector::StartCycle020()
{
    if (ProductionCoordinator)
    {
        ProductionCoordinator->InitializeCycle020Tasks();
        ProductionCoordinator->GenerateProductionReport();
        UE_LOG(LogTemp, Warning, TEXT("Cycle 020 production started - Focus: Playable Prototype"));
    }
}

void ADir_ProductionDirector::EmergencyPrototypeValidation()
{
    if (ProductionCoordinator)
    {
        ProductionCoordinator->ValidatePlayablePrototype();
        ProductionCoordinator->GenerateProductionReport();
        
        if (!ProductionCoordinator->IsPlayablePrototypeReady())
        {
            UE_LOG(LogTemp, Error, TEXT("EMERGENCY: Playable prototype validation FAILED"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("SUCCESS: Playable prototype validation PASSED"));
        }
    }
}