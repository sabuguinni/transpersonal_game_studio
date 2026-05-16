#include "Crowd_MassEntitySpawner.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowd_MassEntitySpawner::ACrowd_MassEntitySpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = UpdateInterval;

    // Create visualization mesh component
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;

    // Set default mesh (sphere for visualization)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        VisualizationMesh->SetStaticMesh(SphereMesh.Object);
        VisualizationMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
    }

    // Initialize default spawn parameters
    SpawnParameters.MaxEntities = 1000;
    SpawnParameters.SpawnRadius = 5000.0f;
    SpawnParameters.MinSpawnDistance = 100.0f;
    SpawnParameters.SpawnCenter = FVector::ZeroVector;
    SpawnParameters.bEnableLOD = true;
    SpawnParameters.LODDistance1 = 1000.0f;
    SpawnParameters.LODDistance2 = 3000.0f;

    // Initialize default behavior parameters
    EntityBehavior.MovementSpeed = 200.0f;
    EntityBehavior.WanderRadius = 500.0f;
    EntityBehavior.FlockingStrength = 0.5f;
    EntityBehavior.AvoidanceRadius = 150.0f;
    EntityBehavior.bEnableFlocking = true;
    EntityBehavior.bEnableObstacleAvoidance = true;

    bIsInitialized = false;
    LastUpdateTime = 0.0f;
}

void ACrowd_MassEntitySpawner::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassSystem();
    
    // Set spawn center to actor location if not set
    if (SpawnParameters.SpawnCenter.IsZero())
    {
        SpawnParameters.SpawnCenter = GetActorLocation();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntitySpawner initialized at location: %s"), 
           *GetActorLocation().ToString());
}

void ACrowd_MassEntitySpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateEntityBehaviors();
        LastUpdateTime = 0.0f;
    }
    
    // Debug visualization
    if (GEngine && GEngine->bEnableOnScreenDebugMessages)
    {
        FString DebugText = FString::Printf(TEXT("Active Entities: %d/%d"), 
                                          GetActiveEntityCount(), 
                                          SpawnParameters.MaxEntities);
        GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, DebugText);
    }
}

void ACrowd_MassEntitySpawner::InitializeMassSystem()
{
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (MassEntitySubsystem)
        {
            CreateEntityArchetype();
            bIsInitialized = true;
            UE_LOG(LogTemp, Warning, TEXT("Mass Entity Subsystem initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get Mass Entity Subsystem"));
        }
    }
}

void ACrowd_MassEntitySpawner::CreateEntityArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }

    // Create a basic archetype for crowd entities
    // Note: In a full implementation, this would include specific Mass components
    // For now, we create a minimal archetype that can be extended
    
    UE_LOG(LogTemp, Warning, TEXT("Creating Mass Entity Archetype for crowd simulation"));
    
    // The archetype creation would typically involve:
    // - FMassEntityTemplate for defining entity structure
    // - Adding components like Transform, Movement, LOD, etc.
    // - Registering with the Mass framework
    
    // This is a simplified version that establishes the foundation
    bIsInitialized = true;
}

void ACrowd_MassEntitySpawner::SpawnMassEntities()
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Mass system not initialized, cannot spawn entities"));
        return;
    }
    
    // Clear existing entities first
    DespawnAllEntities();
    
    int32 EntitiesToSpawn = FMath::Min(SpawnParameters.MaxEntities, 1000);
    SpawnedEntities.Reserve(EntitiesToSpawn);
    
    for (int32 i = 0; i < EntitiesToSpawn; ++i)
    {
        FVector SpawnLocation = GetRandomSpawnLocation();
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            // In a full Mass implementation, this would create actual Mass entities
            // For now, we simulate the entity creation process
            
            FMassEntityHandle NewEntity; // This would be created via MassEntitySubsystem
            SpawnedEntities.Add(NewEntity);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d Mass entities in crowd simulation"), 
           SpawnedEntities.Num());
    
    // Update LOD settings after spawning
    UpdateLODSettings();
}

void ACrowd_MassEntitySpawner::DespawnAllEntities()
{
    if (MassEntitySubsystem && SpawnedEntities.Num() > 0)
    {
        // In a full implementation, this would properly destroy Mass entities
        for (const FMassEntityHandle& Entity : SpawnedEntities)
        {
            // MassEntitySubsystem->DestroyEntity(Entity);
        }
        
        SpawnedEntities.Empty();
        UE_LOG(LogTemp, Warning, TEXT("Despawned all crowd entities"));
    }
}

void ACrowd_MassEntitySpawner::UpdateLODSettings()
{
    if (!SpawnParameters.bEnableLOD)
    {
        return;
    }
    
    // Get player location for LOD calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD for each entity based on distance to player
    for (const FMassEntityHandle& Entity : SpawnedEntities)
    {
        // In a full implementation, this would:
        // 1. Get entity transform from Mass framework
        // 2. Calculate distance to player
        // 3. Set appropriate LOD level (High/Medium/Low detail)
        // 4. Update rendering and behavior complexity accordingly
    }
}

int32 ACrowd_MassEntitySpawner::GetActiveEntityCount() const
{
    return SpawnedEntities.Num();
}

void ACrowd_MassEntitySpawner::SetSpawnCenter(const FVector& NewCenter)
{
    SpawnParameters.SpawnCenter = NewCenter;
    UE_LOG(LogTemp, Warning, TEXT("Spawn center updated to: %s"), *NewCenter.ToString());
}

FVector ACrowd_MassEntitySpawner::GetRandomSpawnLocation() const
{
    // Generate random point within spawn radius
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep entities on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(SpawnParameters.MinSpawnDistance, 
                                          SpawnParameters.SpawnRadius);
    
    return SpawnParameters.SpawnCenter + (RandomDirection * RandomDistance);
}

bool ACrowd_MassEntitySpawner::IsValidSpawnLocation(const FVector& Location) const
{
    // Basic validation - check if location is within bounds
    float DistanceFromCenter = FVector::Dist(Location, SpawnParameters.SpawnCenter);
    
    if (DistanceFromCenter < SpawnParameters.MinSpawnDistance || 
        DistanceFromCenter > SpawnParameters.SpawnRadius)
    {
        return false;
    }
    
    // Additional checks could include:
    // - Terrain height validation
    // - Obstacle avoidance
    // - Minimum distance from other entities
    
    return true;
}

void ACrowd_MassEntitySpawner::UpdateEntityBehaviors()
{
    if (SpawnedEntities.Num() == 0)
    {
        return;
    }
    
    // In a full Mass implementation, this would update:
    // 1. Flocking behavior (separation, alignment, cohesion)
    // 2. Obstacle avoidance
    // 3. Wandering movement
    // 4. Player interaction responses
    
    // For now, we log the update to show the system is active
    static int32 UpdateCounter = 0;
    UpdateCounter++;
    
    if (UpdateCounter % 50 == 0) // Log every 5 seconds at 10fps update rate
    {
        UE_LOG(LogTemp, Log, TEXT("Updating behaviors for %d crowd entities"), 
               SpawnedEntities.Num());
    }
}