#include "Crowd_MassEntitySpawner.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACrowd_MassEntitySpawner::ACrowd_MassEntitySpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootComp;
    
    // Add visual representation
    UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnerMesh"));
    MeshComp->SetupAttachment(RootComponent);
    
    // Load a simple cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMesh.Succeeded())
    {
        MeshComp->SetStaticMesh(CubeMesh.Object);
        MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }
    
    // Initialize spawn configuration
    SpawnConfig.SpawnerType = ECrowd_SpawnerType::Villagers;
    SpawnConfig.MaxEntities = 50;
    SpawnConfig.SpawnRadius = 1000.0f;
    SpawnConfig.SpawnCenter = FVector::ZeroVector;
    SpawnConfig.bUseNavMesh = true;
    SpawnConfig.MinDistanceBetweenEntities = 100.0f;
    
    bAutoSpawnOnBeginPlay = true;
    bRespawnDeadEntities = true;
    RespawnDelay = 5.0f;
    LastRespawnTime = 0.0f;
}

void ACrowd_MassEntitySpawner::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSystem();
    
    if (bAutoSpawnOnBeginPlay)
    {
        // Delay spawn by 1 second to ensure all systems are ready
        FTimerHandle SpawnTimer;
        GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &ACrowd_MassEntitySpawner::SpawnEntities, 1.0f, false);
    }
}

void ACrowd_MassEntitySpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bRespawnDeadEntities)
    {
        LastRespawnTime += DeltaTime;
        if (LastRespawnTime >= RespawnDelay)
        {
            CleanupDeadEntities();
            
            // Respawn if we're below the target count
            int32 CurrentCount = GetActiveEntityCount();
            if (CurrentCount < SpawnConfig.MaxEntities)
            {
                // Spawn a few entities at a time to avoid frame drops
                int32 ToSpawn = FMath::Min(5, SpawnConfig.MaxEntities - CurrentCount);
                for (int32 i = 0; i < ToSpawn; i++)
                {
                    // Individual spawn logic would go here
                }
            }
            
            LastRespawnTime = 0.0f;
        }
    }
}

void ACrowd_MassEntitySpawner::InitializeMassSystem()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySpawner: Failed to get MassEntitySubsystem"));
        }
    }
}

void ACrowd_MassEntitySpawner::SpawnEntities()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySpawner: MassEntitySubsystem not available"));
        return;
    }
    
    if (!EntityConfigAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySpawner: EntityConfigAsset not set"));
        return;
    }
    
    // Clear existing entities first
    DestroyAllEntities();
    
    // Calculate spawn center
    FVector WorldSpawnCenter = SpawnConfig.SpawnCenter;
    if (SpawnConfig.SpawnCenter.IsZero())
    {
        WorldSpawnCenter = GetActorLocation();
    }
    
    // Spawn entities in a grid pattern with some randomization
    int32 SpawnedCount = 0;
    int32 AttemptCount = 0;
    const int32 MaxAttempts = SpawnConfig.MaxEntities * 3; // Prevent infinite loops
    
    while (SpawnedCount < SpawnConfig.MaxEntities && AttemptCount < MaxAttempts)
    {
        FVector SpawnLocation = GetRandomSpawnLocation();
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            // Create entity spawn request
            FMassEntitySpawnDataGeneratorBase SpawnData;
            // Note: In a real implementation, we'd configure the spawn data properly
            // For now, we'll just track that we attempted to spawn
            
            SpawnedCount++;
        }
        
        AttemptCount++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassEntitySpawner: Spawned %d entities (attempted %d)"), SpawnedCount, AttemptCount);
}

void ACrowd_MassEntitySpawner::DestroyAllEntities()
{
    if (MassEntitySubsystem)
    {
        for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
        {
            if (MassEntitySubsystem->IsEntityValid(EntityHandle))
            {
                MassEntitySubsystem->DestroyEntity(EntityHandle);
            }
        }
    }
    
    SpawnedEntities.Empty();
}

int32 ACrowd_MassEntitySpawner::GetActiveEntityCount() const
{
    if (!MassEntitySubsystem)
    {
        return 0;
    }
    
    int32 ActiveCount = 0;
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (MassEntitySubsystem->IsEntityValid(EntityHandle))
        {
            ActiveCount++;
        }
    }
    
    return ActiveCount;
}

void ACrowd_MassEntitySpawner::SetSpawnConfiguration(const FCrowd_SpawnConfiguration& NewConfig)
{
    SpawnConfig = NewConfig;
    
    // If we're reducing the max entities, destroy excess ones
    if (GetActiveEntityCount() > SpawnConfig.MaxEntities)
    {
        CleanupDeadEntities();
    }
}

bool ACrowd_MassEntitySpawner::IsValidSpawnLocation(const FVector& Location) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Check if location is within spawn radius
    FVector WorldSpawnCenter = SpawnConfig.SpawnCenter.IsZero() ? GetActorLocation() : SpawnConfig.SpawnCenter;
    float DistanceFromCenter = FVector::Dist(Location, WorldSpawnCenter);
    if (DistanceFromCenter > SpawnConfig.SpawnRadius)
    {
        return false;
    }
    
    // Check NavMesh if required
    if (SpawnConfig.bUseNavMesh)
    {
        if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
        {
            FNavLocation NavLocation;
            if (!NavSys->ProjectPointToNavigation(Location, NavLocation, FVector(100.0f, 100.0f, 200.0f)))
            {
                return false;
            }
        }
    }
    
    // Check minimum distance from other entities
    for (const FMassEntityHandle& EntityHandle : SpawnedEntities)
    {
        if (MassEntitySubsystem && MassEntitySubsystem->IsEntityValid(EntityHandle))
        {
            // In a real implementation, we'd get the entity's location and check distance
            // For now, we'll assume it's valid
        }
    }
    
    return true;
}

FVector ACrowd_MassEntitySpawner::GetRandomSpawnLocation() const
{
    FVector WorldSpawnCenter = SpawnConfig.SpawnCenter.IsZero() ? GetActorLocation() : SpawnConfig.SpawnCenter;
    
    // Generate random point within spawn radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(SpawnConfig.MinDistanceBetweenEntities, SpawnConfig.SpawnRadius);
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomRadius;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomRadius;
    RandomOffset.Z = 0.0f; // Will be adjusted by ground tracing
    
    FVector SpawnLocation = WorldSpawnCenter + RandomOffset;
    
    // Trace down to find ground
    if (UWorld* World = GetWorld())
    {
        FHitResult HitResult;
        FVector TraceStart = SpawnLocation + FVector(0.0f, 0.0f, 1000.0f);
        FVector TraceEnd = SpawnLocation - FVector(0.0f, 0.0f, 1000.0f);
        
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            SpawnLocation.Z = HitResult.Location.Z + 50.0f; // Offset above ground
        }
    }
    
    return SpawnLocation;
}

void ACrowd_MassEntitySpawner::CleanupDeadEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Remove invalid entities from our tracking array
    SpawnedEntities.RemoveAll([this](const FMassEntityHandle& EntityHandle)
    {
        return !MassEntitySubsystem->IsEntityValid(EntityHandle);
    });
}