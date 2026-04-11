#include "MassEntityCrowdManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"

AMassEntityCrowdManager::AMassEntityCrowdManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for crowd updates

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    bCrowdActive = false;
    CurrentAgentCount = 0;
    CurrentBehavior = ECrowdBehaviorType::Gathering;
    PlayerLocation = FVector::ZeroVector;
    LastFrameTime = 0.0f;
    VisibleAgentCount = 0;

    // Set default configuration
    DefaultConfig.BehaviorType = ECrowdBehaviorType::Gathering;
    DefaultConfig.DensityLevel = ECrowdDensityLevel::Medium;
    DefaultConfig.MaxAgents = 200;
    DefaultConfig.SpawnRadius = 500.0f;
    DefaultConfig.bUseNavMesh = true;
    DefaultConfig.MovementSpeed = 150.0f;
    DefaultConfig.LODDistance1 = 1000.0f;
    DefaultConfig.LODDistance2 = 2500.0f;
    DefaultConfig.LODDistance3 = 5000.0f;
}

void AMassEntityCrowdManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: BeginPlay started"));

    // Initialize Mass Entity systems
    InitializeMassEntitySystems();

    // Set up timers for periodic updates
    GetWorldTimerManager().SetTimer(LODUpdateTimer, this, &AMassEntityCrowdManager::UpdateAgentLOD, 0.5f, true);
    GetWorldTimerManager().SetTimer(PerformanceUpdateTimer, this, &AMassEntityCrowdManager::UpdatePerformanceMetrics, 1.0f, true);

    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: BeginPlay completed"));
}

void AMassEntityCrowdManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up timers
    GetWorldTimerManager().ClearTimer(LODUpdateTimer);
    GetWorldTimerManager().ClearTimer(BehaviorUpdateTimer);
    GetWorldTimerManager().ClearTimer(PerformanceUpdateTimer);

    // Despawn all crowd entities
    DespawnCrowd();

    Super::EndPlay(EndPlayReason);
}

void AMassEntityCrowdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastFrameTime = DeltaTime;

    if (bCrowdActive)
    {
        ProcessCrowdBehavior(DeltaTime);
    }
}

void AMassEntityCrowdManager::InitializeMassEntitySystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntityCrowdManager: No world available"));
        return;
    }

    // Get Mass Entity Subsystem
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntityCrowdManager: Mass Entity Subsystem not available"));
        return;
    }

    // Get Mass Spawner Subsystem
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Mass Spawner Subsystem not available"));
    }

    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Mass Entity systems initialized"));
}

void AMassEntityCrowdManager::SpawnCrowd(const FCrowdSpawnConfiguration& Config)
{
    if (!ValidateSpawnConfiguration(Config))
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntityCrowdManager: Invalid spawn configuration"));
        return;
    }

    // Despawn existing crowd first
    if (bCrowdActive)
    {
        DespawnCrowd();
    }

    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Spawning crowd with %d agents"), Config.MaxAgents);

    // Create crowd entities
    CreateCrowdEntities(Config);

    // Update state
    bCrowdActive = true;
    CurrentBehavior = Config.BehaviorType;

    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Crowd spawned successfully"));
}

void AMassEntityCrowdManager::CreateCrowdEntities(const FCrowdSpawnConfiguration& Config)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntityCrowdManager: Mass Entity Subsystem not available for spawning"));
        return;
    }

    // Clear existing entities
    SpawnedEntities.Empty();

    // Calculate spawn positions
    TArray<FVector> SpawnPositions;
    for (int32 i = 0; i < Config.MaxAgents; i++)
    {
        // Generate random position within spawn radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Config.SpawnRadius);
        
        FVector SpawnOffset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );

        FVector SpawnPosition = Config.SpawnCenter + SpawnOffset;
        
        // If using NavMesh, project to ground
        if (Config.bUseNavMesh)
        {
            UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
            if (NavSys)
            {
                FNavLocation NavLocation;
                if (NavSys->ProjectPointToNavigation(SpawnPosition, NavLocation, FVector(100.0f, 100.0f, 500.0f)))
                {
                    SpawnPosition = NavLocation.Location;
                }
            }
        }

        SpawnPositions.Add(SpawnPosition);
    }

    // Create Mass Entities (simplified approach for now)
    for (int32 i = 0; i < SpawnPositions.Num(); i++)
    {
        // Note: This is a simplified implementation
        // In a full Mass Entity setup, we would create entities with proper fragments
        // For now, we'll track the intended entities
        FMassEntityHandle EntityHandle;
        EntityHandle.Index = i;
        EntityHandle.SerialNumber = 1;
        SpawnedEntities.Add(EntityHandle);
    }

    CurrentAgentCount = SpawnedEntities.Num();
    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Created %d crowd entities"), CurrentAgentCount);
}

void AMassEntityCrowdManager::DespawnCrowd()
{
    if (!bCrowdActive)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Despawning crowd"));

    CleanupCrowdEntities();

    bCrowdActive = false;
    CurrentAgentCount = 0;
    SpawnedEntities.Empty();

    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Crowd despawned"));
}

void AMassEntityCrowdManager::CleanupCrowdEntities()
{
    // Clean up Mass Entities
    if (MassEntitySubsystem && SpawnedEntities.Num() > 0)
    {
        // Note: In a full implementation, we would properly destroy Mass Entities here
        // For now, we just clear our tracking array
        SpawnedEntities.Empty();
    }
}

void AMassEntityCrowdManager::UpdateCrowdBehavior(ECrowdBehaviorType NewBehavior)
{
    if (CurrentBehavior == NewBehavior)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Updating crowd behavior to %d"), (int32)NewBehavior);

    CurrentBehavior = NewBehavior;

    // Implement behavior change
    switch (NewBehavior)
    {
        case ECrowdBehaviorType::Gathering:
            ImplementGatheringBehavior();
            break;
        case ECrowdBehaviorType::Commerce:
            ImplementCommerceBehavior();
            break;
        case ECrowdBehaviorType::Residential:
            ImplementResidentialBehavior();
            break;
        case ECrowdBehaviorType::Work:
            ImplementWorkBehavior();
            break;
        case ECrowdBehaviorType::Ceremony:
            ImplementCeremonyBehavior();
            break;
        case ECrowdBehaviorType::Patrol:
            ImplementPatrolBehavior();
            break;
        case ECrowdBehaviorType::Flee:
            ImplementFleeBehavior();
            break;
    }
}

void AMassEntityCrowdManager::SetCrowdDensity(ECrowdDensityLevel NewDensity)
{
    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Setting crowd density to %d"), (int32)NewDensity);

    // Adjust agent count based on density level
    int32 NewMaxAgents = 0;
    switch (NewDensity)
    {
        case ECrowdDensityLevel::Sparse:
            NewMaxAgents = 10;
            break;
        case ECrowdDensityLevel::Light:
            NewMaxAgents = 50;
            break;
        case ECrowdDensityLevel::Medium:
            NewMaxAgents = 200;
            break;
        case ECrowdDensityLevel::Dense:
            NewMaxAgents = 1000;
            break;
        case ECrowdDensityLevel::Massive:
            NewMaxAgents = 5000;
            break;
    }

    // Update configuration and respawn if necessary
    DefaultConfig.MaxAgents = NewMaxAgents;
    DefaultConfig.DensityLevel = NewDensity;

    if (bCrowdActive)
    {
        SpawnCrowd(DefaultConfig);
    }
}

int32 AMassEntityCrowdManager::GetActiveAgentCount() const
{
    return CurrentAgentCount;
}

void AMassEntityCrowdManager::PauseCrowdSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Pausing crowd simulation"));
    SetActorTickEnabled(false);
}

void AMassEntityCrowdManager::ResumeCrowdSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Resuming crowd simulation"));
    SetActorTickEnabled(true);
}

void AMassEntityCrowdManager::UpdateLODDistances(float LOD1, float LOD2, float LOD3)
{
    DefaultConfig.LODDistance1 = LOD1;
    DefaultConfig.LODDistance2 = LOD2;
    DefaultConfig.LODDistance3 = LOD3;

    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Updated LOD distances: %.1f, %.1f, %.1f"), LOD1, LOD2, LOD3);
}

void AMassEntityCrowdManager::SetPlayerLocation(const FVector& PlayerPos)
{
    PlayerLocation = PlayerPos;
}

void AMassEntityCrowdManager::TriggerCrowdReaction(const FVector& EventLocation, float EventRadius, ECrowdBehaviorType ReactionType)
{
    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Triggering crowd reaction at location (%.1f, %.1f, %.1f) with radius %.1f"), 
           EventLocation.X, EventLocation.Y, EventLocation.Z, EventRadius);

    // Implement crowd reaction logic here
    UpdateCrowdBehavior(ReactionType);
}

void AMassEntityCrowdManager::SetCrowdFleeTarget(const FVector& FleeFromLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Setting flee target at (%.1f, %.1f, %.1f)"), 
           FleeFromLocation.X, FleeFromLocation.Y, FleeFromLocation.Z);

    UpdateCrowdBehavior(ECrowdBehaviorType::Flee);
}

float AMassEntityCrowdManager::GetCurrentFrameTime() const
{
    return LastFrameTime;
}

int32 AMassEntityCrowdManager::GetVisibleAgentCount() const
{
    return VisibleAgentCount;
}

void AMassEntityCrowdManager::UpdateAgentLOD()
{
    if (!bCrowdActive || PlayerLocation.IsZero())
    {
        return;
    }

    // Update LOD based on distance to player
    VisibleAgentCount = 0;
    
    // In a full implementation, we would iterate through all Mass Entities
    // and update their LOD based on distance to player
    for (int32 i = 0; i < SpawnedEntities.Num(); i++)
    {
        // Simplified LOD calculation
        // In reality, we would get the actual entity position
        float DistanceToPlayer = 1000.0f; // Placeholder
        
        if (DistanceToPlayer < DefaultConfig.LODDistance1)
        {
            VisibleAgentCount++;
        }
    }
}

void AMassEntityCrowdManager::ProcessCrowdBehavior(float DeltaTime)
{
    if (!bCrowdActive)
    {
        return;
    }

    // Process current behavior
    switch (CurrentBehavior)
    {
        case ECrowdBehaviorType::Gathering:
            ImplementGatheringBehavior();
            break;
        case ECrowdBehaviorType::Commerce:
            ImplementCommerceBehavior();
            break;
        case ECrowdBehaviorType::Residential:
            ImplementResidentialBehavior();
            break;
        case ECrowdBehaviorType::Work:
            ImplementWorkBehavior();
            break;
        case ECrowdBehaviorType::Ceremony:
            ImplementCeremonyBehavior();
            break;
        case ECrowdBehaviorType::Patrol:
            ImplementPatrolBehavior();
            break;
        case ECrowdBehaviorType::Flee:
            ImplementFleeBehavior();
            break;
    }
}

void AMassEntityCrowdManager::ImplementGatheringBehavior()
{
    // Agents move towards center and form loose groups
    UE_LOG(LogTemp, Verbose, TEXT("MassEntityCrowdManager: Implementing gathering behavior"));
}

void AMassEntityCrowdManager::ImplementCommerceBehavior()
{
    // Agents move between market stalls and form queues
    UE_LOG(LogTemp, Verbose, TEXT("MassEntityCrowdManager: Implementing commerce behavior"));
}

void AMassEntityCrowdManager::ImplementResidentialBehavior()
{
    // Agents move between homes and common areas
    UE_LOG(LogTemp, Verbose, TEXT("MassEntityCrowdManager: Implementing residential behavior"));
}

void AMassEntityCrowdManager::ImplementWorkBehavior()
{
    // Agents move to work areas and perform tasks
    UE_LOG(LogTemp, Verbose, TEXT("MassEntityCrowdManager: Implementing work behavior"));
}

void AMassEntityCrowdManager::ImplementCeremonyBehavior()
{
    // Agents form circles and perform ritual movements
    UE_LOG(LogTemp, Verbose, TEXT("MassEntityCrowdManager: Implementing ceremony behavior"));
}

void AMassEntityCrowdManager::ImplementPatrolBehavior()
{
    // Agents follow patrol routes
    UE_LOG(LogTemp, Verbose, TEXT("MassEntityCrowdManager: Implementing patrol behavior"));
}

void AMassEntityCrowdManager::ImplementFleeBehavior()
{
    // Agents flee from danger source
    UE_LOG(LogTemp, Verbose, TEXT("MassEntityCrowdManager: Implementing flee behavior"));
}

void AMassEntityCrowdManager::OptimizeForPerformance()
{
    // Implement performance optimizations
    if (LastFrameTime > 0.033f) // If frame time exceeds 30 FPS
    {
        // Reduce agent count or increase LOD distances
        UE_LOG(LogTemp, Warning, TEXT("MassEntityCrowdManager: Performance optimization triggered"));
    }
}

void AMassEntityCrowdManager::UpdatePerformanceMetrics()
{
    OptimizeForPerformance();
    
    // Log performance metrics
    UE_LOG(LogTemp, Verbose, TEXT("MassEntityCrowdManager: Frame time: %.3f ms, Active agents: %d, Visible agents: %d"), 
           LastFrameTime * 1000.0f, CurrentAgentCount, VisibleAgentCount);
}

bool AMassEntityCrowdManager::ValidateSpawnConfiguration(const FCrowdSpawnConfiguration& Config) const
{
    if (Config.MaxAgents <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntityCrowdManager: Invalid MaxAgents: %d"), Config.MaxAgents);
        return false;
    }

    if (Config.SpawnRadius <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntityCrowdManager: Invalid SpawnRadius: %.1f"), Config.SpawnRadius);
        return false;
    }

    return true;
}