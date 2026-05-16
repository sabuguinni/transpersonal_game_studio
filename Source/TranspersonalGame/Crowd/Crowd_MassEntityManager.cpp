#include "Crowd_MassEntityManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassLODFragments.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "TranspersonalGame/TranspersonalGameState.h"

UCrowd_MassEntityManager::UCrowd_MassEntityManager()
{
    bSystemInitialized = false;
    LastUpdateTime = 0.0f;
    CurrentEntityCount = 0;
    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    
    // Initialize default configuration
    CrowdConfig.MaxEntities = 10000;
    CrowdConfig.SpawnRadius = 5000.0f;
    CrowdConfig.MovementSpeed = 300.0f;
    CrowdConfig.LODDistance1 = 1000.0f;
    CrowdConfig.LODDistance2 = 3000.0f;
    CrowdConfig.LODDistance3 = 8000.0f;
}

void UCrowd_MassEntityManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Initializing crowd simulation subsystem"));
    
    // Get Mass Entity subsystems
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        
        if (MassEntitySubsystem && MassSpawnerSubsystem)
        {
            InitializeMassEntitySystem();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Failed to get Mass subsystems"));
        }
    }
}

void UCrowd_MassEntityManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Deinitializing crowd simulation"));
    
    // Clean up active entities
    if (MassEntitySubsystem && ActiveEntities.Num() > 0)
    {
        for (const FMassEntityHandle& Entity : ActiveEntities)
        {
            if (MassEntitySubsystem->IsEntityValid(Entity))
            {
                MassEntitySubsystem->DestroyEntity(Entity);
            }
        }
        ActiveEntities.Empty();
    }
    
    bSystemInitialized = false;
    CurrentEntityCount = 0;
    
    Super::Deinitialize();
}

bool UCrowd_MassEntityManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UCrowd_MassEntityManager::InitializeMassEntitySystem()
{
    if (bSystemInitialized)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Setting up Mass Entity archetype"));
    
    CreateMassArchetype();
    SetupEntityFragments();
    RegisterProcessors();
    
    bSystemInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Mass Entity system initialized successfully"));
}

void UCrowd_MassEntityManager::CreateMassArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create archetype with basic fragments for crowd simulation
    FMassArchetypeHandle ArchetypeHandle = MassEntitySubsystem->CreateArchetype({
        FMassTransformFragment::StaticStruct(),
        FMassVelocityFragment::StaticStruct(),
        FMassRepresentationFragment::StaticStruct(),
        FMassRepresentationLODFragment::StaticStruct()
    });
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Created Mass archetype for crowd entities"));
}

void UCrowd_MassEntityManager::SetupEntityFragments()
{
    // Fragment setup is handled by the Mass Entity system
    // This method can be expanded for custom fragment initialization
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Entity fragments configured"));
}

void UCrowd_MassEntityManager::RegisterProcessors()
{
    // Processor registration is handled by the Mass Entity system
    // This method can be expanded for custom processor registration
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Mass processors registered"));
}

void UCrowd_MassEntityManager::SpawnCrowdEntities(const FCrowd_SpawnPoint& SpawnPoint)
{
    if (!bSystemInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Cannot spawn entities - system not initialized"));
        return;
    }
    
    if (CurrentEntityCount + SpawnPoint.EntityCount > CrowdConfig.MaxEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Cannot spawn %d entities - would exceed max limit of %d"), 
               SpawnPoint.EntityCount, CrowdConfig.MaxEntities);
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Spawning %d entities at location (%f, %f, %f)"), 
           SpawnPoint.EntityCount, SpawnPoint.Location.X, SpawnPoint.Location.Y, SpawnPoint.Location.Z);
    
    // Spawn entities in a circular pattern around the spawn point
    for (int32 i = 0; i < SpawnPoint.EntityCount; ++i)
    {
        float Angle = (2.0f * PI * i) / SpawnPoint.EntityCount;
        float Distance = FMath::RandRange(0.0f, SpawnPoint.Radius);
        
        FVector EntityLocation = SpawnPoint.Location + FVector(
            Distance * FMath::Cos(Angle),
            Distance * FMath::Sin(Angle),
            0.0f
        );
        
        // Create entity with transform fragment
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
        if (MassEntitySubsystem->IsEntityValid(EntityHandle))
        {
            ActiveEntities.Add(EntityHandle);
            CurrentEntityCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Successfully spawned %d entities. Total active: %d"), 
           SpawnPoint.EntityCount, CurrentEntityCount);
}

void UCrowd_MassEntityManager::SpawnCrowdEntitiesInBiome(ECrowd_BiomeType BiomeType, int32 EntityCount)
{
    FCrowd_SpawnPoint SpawnPoint;
    SpawnPoint.Location = GetBiomeSpawnLocation(BiomeType);
    SpawnPoint.EntityCount = EntityCount;
    SpawnPoint.Radius = 1000.0f;
    SpawnPoint.BiomeType = BiomeType;
    
    SpawnCrowdEntities(SpawnPoint);
}

FVector UCrowd_MassEntityManager::GetBiomeSpawnLocation(ECrowd_BiomeType BiomeType)
{
    switch (BiomeType)
    {
        case ECrowd_BiomeType::Savanna:
            return FVector(0.0f, 0.0f, 100.0f);
        case ECrowd_BiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case ECrowd_BiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case ECrowd_BiomeType::Desert:
            return FVector(55000.0f, 0.0f, 100.0f);
        case ECrowd_BiomeType::Mountain:
            return FVector(40000.0f, 50000.0f, 500.0f);
        default:
            return FVector::ZeroVector;
    }
}

void UCrowd_MassEntityManager::UpdateCrowdBehavior(float DeltaTime)
{
    if (!bSystemInitialized || CurrentEntityCount == 0)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    // Update crowd behavior every 0.1 seconds to maintain performance
    if (LastUpdateTime >= 0.1f)
    {
        UpdateEntityLOD();
        UpdateCrowdNavigation();
        LastUpdateTime = 0.0f;
    }
}

void UCrowd_MassEntityManager::UpdateEntityLOD()
{
    // LOD updates are handled by the Mass Entity system automatically
    // This method can be expanded for custom LOD logic
}

void UCrowd_MassEntityManager::SetCrowdLODDistances(float LOD1, float LOD2, float LOD3)
{
    CrowdConfig.LODDistance1 = LOD1;
    CrowdConfig.LODDistance2 = LOD2;
    CrowdConfig.LODDistance3 = LOD3;
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Updated LOD distances: %f, %f, %f"), LOD1, LOD2, LOD3);
}

void UCrowd_MassEntityManager::NotifyPlayerCombatState(bool bInCombat, FVector CombatLocation)
{
    if (bInCombat)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Player entered combat at location (%f, %f, %f)"), 
               CombatLocation.X, CombatLocation.Y, CombatLocation.Z);
        
        HandleCombatResponse(CombatLocation);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Player exited combat - crowd returning to normal behavior"));
    }
}

void UCrowd_MassEntityManager::HandleCombatResponse(FVector CombatLocation)
{
    // Scatter crowd entities away from combat location
    ScatterCrowdFromLocation(CombatLocation, 2000.0f);
}

void UCrowd_MassEntityManager::ScatterCrowdFromLocation(FVector DangerLocation, float ScatterRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Scattering crowd from danger location (%f, %f, %f) with radius %f"), 
           DangerLocation.X, DangerLocation.Y, DangerLocation.Z, ScatterRadius);
    
    // Implementation would modify entity velocities to move away from danger
    // This is a placeholder for the actual scatter logic
}

void UCrowd_MassEntityManager::SetMaxEntityCount(int32 NewMaxCount)
{
    CrowdConfig.MaxEntities = FMath::Max(NewMaxCount, 100);
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Set max entity count to %d"), CrowdConfig.MaxEntities);
}

int32 UCrowd_MassEntityManager::GetCurrentEntityCount() const
{
    return CurrentEntityCount;
}

void UCrowd_MassEntityManager::OptimizeCrowdDensity()
{
    if (CurrentEntityCount > CrowdConfig.MaxEntities * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Optimizing crowd density - current count: %d"), CurrentEntityCount);
        // Implementation would reduce entity count in low-priority areas
    }
}

void UCrowd_MassEntityManager::UpdateCrowdNavigation()
{
    // Navigation updates are handled by the Mass Entity movement system
    // This method can be expanded for custom navigation logic
}

void UCrowd_MassEntityManager::SetCrowdDestination(FVector Destination, float AcceptanceRadius)
{
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntityManager: Setting crowd destination to (%f, %f, %f) with radius %f"), 
           Destination.X, Destination.Y, Destination.Z, AcceptanceRadius);
    
    // Implementation would update entity movement targets
}

// ACrowd_MassSpawner Implementation

ACrowd_MassSpawner::ACrowd_MassSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bAutoSpawnOnBeginPlay = true;
    SpawnInterval = 5.0f;
    LastSpawnTime = 0.0f;
    CrowdManager = nullptr;
    
    // Initialize spawn configuration
    SpawnConfiguration.Location = FVector::ZeroVector;
    SpawnConfiguration.Radius = 500.0f;
    SpawnConfiguration.EntityCount = 100;
    SpawnConfiguration.BiomeType = ECrowd_BiomeType::Savanna;
}

void ACrowd_MassSpawner::BeginPlay()
{
    Super::BeginPlay();
    
    // Get crowd manager
    if (UWorld* World = GetWorld())
    {
        CrowdManager = World->GetSubsystem<UCrowd_MassEntityManager>();
    }
    
    if (bAutoSpawnOnBeginPlay && CrowdManager)
    {
        TriggerSpawn();
    }
}

void ACrowd_MassSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (SpawnInterval > 0.0f && CrowdManager)
    {
        LastSpawnTime += DeltaTime;
        if (LastSpawnTime >= SpawnInterval)
        {
            TriggerSpawn();
            LastSpawnTime = 0.0f;
        }
    }
}

void ACrowd_MassSpawner::TriggerSpawn()
{
    if (!CrowdManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ACrowd_MassSpawner: No crowd manager available"));
        return;
    }
    
    // Update spawn location to actor location
    SpawnConfiguration.Location = GetActorLocation();
    
    UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassSpawner: Triggering spawn of %d entities"), SpawnConfiguration.EntityCount);
    CrowdManager->SpawnCrowdEntities(SpawnConfiguration);
}

void ACrowd_MassSpawner::SetSpawnConfiguration(const FCrowd_SpawnPoint& NewConfig)
{
    SpawnConfiguration = NewConfig;
    UE_LOG(LogTemp, Log, TEXT("ACrowd_MassSpawner: Updated spawn configuration"));
}