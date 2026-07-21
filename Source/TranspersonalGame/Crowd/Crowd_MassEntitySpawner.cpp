#include "Crowd_MassEntitySpawner.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ACrowd_MassEntitySpawner::ACrowd_MassEntitySpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Create spawner mesh component
    SpawnerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnerMesh"));
    RootComponent = SpawnerMesh;

    // Initialize default values
    LastSpawnTime = 0.0f;
    bMassSystemInitialized = false;
    MassEntitySubsystem = nullptr;

    // Set default spawn settings
    SpawnSettings.MaxEntities = 1000;
    SpawnSettings.SpawnRadius = 5000.0f;
    SpawnSettings.MinSpawnDistance = 100.0f;
    SpawnSettings.bSpawnOnBeginPlay = true;
}

void ACrowd_MassEntitySpawner::BeginPlay()
{
    Super::BeginPlay();

    InitializeMassSystem();

    if (SpawnSettings.bSpawnOnBeginPlay && bMassSystemInitialized)
    {
        // Delay spawn by 2 seconds to ensure world is fully loaded
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &ACrowd_MassEntitySpawner::SpawnCrowdEntities,
            2.0f,
            false
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySpawner BeginPlay - MaxEntities: %d, SpawnRadius: %f"), 
           SpawnSettings.MaxEntities, SpawnSettings.SpawnRadius);
}

void ACrowd_MassEntitySpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Clean up invalid entities every 5 seconds
    if (GetWorld()->GetTimeSeconds() - LastSpawnTime > 5.0f)
    {
        CleanupInvalidEntities();
        LastSpawnTime = GetWorld()->GetTimeSeconds();
    }

    // Debug draw spawn radius
    if (GEngine && GEngine->bEnableOnScreenDebugMessages)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), SpawnSettings.SpawnRadius, 32, FColor::Green, false, 0.1f);
    }
}

void ACrowd_MassEntitySpawner::InitializeMassSystem()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            bMassSystemInitialized = true;
            UE_LOG(LogTemp, Warning, TEXT("Mass Entity Subsystem initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get Mass Entity Subsystem"));
            bMassSystemInitialized = false;
        }
    }
}

void ACrowd_MassEntitySpawner::SpawnCrowdEntities()
{
    if (!bMassSystemInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Mass system not initialized, cannot spawn entities"));
        return;
    }

    // Clear existing entities first
    DespawnAllEntities();

    int32 SpawnedCount = 0;
    int32 AttemptCount = 0;
    const int32 MaxAttempts = SpawnSettings.MaxEntities * 3; // Prevent infinite loops

    while (SpawnedCount < SpawnSettings.MaxEntities && AttemptCount < MaxAttempts)
    {
        FVector SpawnLocation = GetRandomSpawnLocation();
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            // Create entity using Mass Entity system
            FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
            
            if (NewEntity.IsValid())
            {
                SpawnedEntities.Add(NewEntity);
                SpawnedCount++;
                
                UE_LOG(LogTemp, Log, TEXT("Spawned crowd entity %d at location %s"), 
                       SpawnedCount, *SpawnLocation.ToString());
            }
        }
        
        AttemptCount++;
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd spawning complete: %d/%d entities spawned in %d attempts"), 
           SpawnedCount, SpawnSettings.MaxEntities, AttemptCount);

    // Broadcast spawn completion
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, 
            FString::Printf(TEXT("Crowd Spawner: %d entities active"), SpawnedCount));
    }
}

void ACrowd_MassEntitySpawner::DespawnAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    int32 DespawnedCount = 0;
    for (const FMassEntityHandle& Entity : SpawnedEntities)
    {
        if (Entity.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(Entity);
            DespawnedCount++;
        }
    }

    SpawnedEntities.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Despawned %d crowd entities"), DespawnedCount);
}

int32 ACrowd_MassEntitySpawner::GetActiveEntityCount() const
{
    int32 ActiveCount = 0;
    for (const FMassEntityHandle& Entity : SpawnedEntities)
    {
        if (Entity.IsValid())
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

void ACrowd_MassEntitySpawner::TestSpawnSingleEntity()
{
    if (!bMassSystemInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Mass system not initialized for test spawn"));
        return;
    }

    FVector TestLocation = GetActorLocation() + FVector(200.0f, 0.0f, 0.0f);
    FMassEntityHandle TestEntity = MassEntitySubsystem->CreateEntity();
    
    if (TestEntity.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Test entity spawned successfully at %s"), *TestLocation.ToString());
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                TEXT("Test crowd entity spawned!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn test entity"));
    }
}

FVector ACrowd_MassEntitySpawner::GetRandomSpawnLocation() const
{
    FVector BaseLocation = GetActorLocation();
    
    // Generate random point within spawn radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(SpawnSettings.MinSpawnDistance, SpawnSettings.SpawnRadius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return BaseLocation + RandomOffset;
}

bool ACrowd_MassEntitySpawner::IsValidSpawnLocation(const FVector& Location) const
{
    if (!GetWorld())
    {
        return false;
    }

    // Check if location is too close to existing entities
    for (const FMassEntityHandle& Entity : SpawnedEntities)
    {
        if (Entity.IsValid())
        {
            // For now, just check minimum distance from spawner
            // In a full implementation, you'd check actual entity positions
            float DistanceToSpawner = FVector::Dist(Location, GetActorLocation());
            if (DistanceToSpawner < SpawnSettings.MinSpawnDistance)
            {
                return false;
            }
        }
    }

    // Basic ground check - trace down to find ground
    FHitResult HitResult;
    FVector TraceStart = Location + FVector(0.0f, 0.0f, 1000.0f);
    FVector TraceEnd = Location - FVector(0.0f, 0.0f, 1000.0f);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_WorldStatic,
        QueryParams
    );

    return bHit; // Valid if we hit ground
}

void ACrowd_MassEntitySpawner::CleanupInvalidEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    int32 CleanedCount = 0;
    for (int32 i = SpawnedEntities.Num() - 1; i >= 0; i--)
    {
        if (!SpawnedEntities[i].IsValid())
        {
            SpawnedEntities.RemoveAt(i);
            CleanedCount++;
        }
    }

    if (CleanedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d invalid entities"), CleanedCount);
    }
}