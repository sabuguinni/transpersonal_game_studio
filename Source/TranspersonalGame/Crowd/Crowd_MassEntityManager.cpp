#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentEntityCount = 0;
    CrowdDensityMultiplier = 1.0f;
    LastFrameTime = 0.0f;
    
    // Set default entity configuration
    EntityConfig.MaxEntities = 1000;
    EntityConfig.SpawnRadius = 5000.0f;
    EntityConfig.MovementSpeed = 300.0f;
    EntityConfig.FlockingRadius = 500.0f;
    EntityConfig.AvoidanceRadius = 200.0f;
    EntityConfig.LODDistance1 = 1000.0f;
    EntityConfig.LODDistance2 = 2000.0f;
    EntityConfig.LODDistance3 = 5000.0f;
    
    // Set default behavior
    DefaultBehavior.WanderRadius = 1000.0f;
    DefaultBehavior.FlockWeight = 1.0f;
    DefaultBehavior.AvoidanceWeight = 2.0f;
    DefaultBehavior.SeekWeight = 0.5f;
    DefaultBehavior.bIsActive = true;
    
    // Initialize subsystem pointers to null
    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    MassSimulationSubsystem = nullptr;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize Mass Entity system
    InitializeMassEntitySystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: BeginPlay - Mass Entity system initialized"));
}

void ACrowd_MassEntityManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all entities before ending play
    DestroyAllMassEntities();
    
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastFrameTime = DeltaTime;
    
    // Update LOD system
    UpdateEntityLOD(DeltaTime);
    
    // Process flocking behavior
    ProcessFlockingBehavior(DeltaTime);
    
    // Validate subsystems periodically
    static float ValidationTimer = 0.0f;
    ValidationTimer += DeltaTime;
    if (ValidationTimer > 5.0f)
    {
        ValidateMassEntitySubsystems();
        ValidationTimer = 0.0f;
    }
}

void ACrowd_MassEntityManager::InitializeMassEntitySystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: No valid world found"));
        return;
    }
    
    // Get Mass Entity subsystems
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: MassEntitySubsystem not available - using fallback"));
    }
    
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: MassSpawnerSubsystem not available - using fallback"));
    }
    
    if (!MassSimulationSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: MassSimulationSubsystem not available - using fallback"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Mass Entity subsystems initialized"));
}

void ACrowd_MassEntityManager::SpawnMassEntities(int32 NumEntities, FVector SpawnCenter)
{
    if (NumEntities <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Invalid entity count: %d"), NumEntities);
        return;
    }
    
    // Clamp to maximum entities
    NumEntities = FMath::Min(NumEntities, EntityConfig.MaxEntities);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Spawning %d entities at location %s"), 
           NumEntities, *SpawnCenter.ToString());
    
    // Clear existing entities
    ManagedEntities.Empty();
    
    // If Mass Entity subsystem is available, use it
    if (MassEntitySubsystem)
    {
        // Create entities using Mass Entity framework
        for (int32 i = 0; i < NumEntities; i++)
        {
            // Generate random spawn position within radius
            FVector RandomOffset = FVector(
                FMath::RandRange(-EntityConfig.SpawnRadius, EntityConfig.SpawnRadius),
                FMath::RandRange(-EntityConfig.SpawnRadius, EntityConfig.SpawnRadius),
                0.0f
            );
            
            FVector SpawnLocation = SpawnCenter + RandomOffset;
            
            // Create Mass Entity (placeholder - would use actual Mass Entity API)
            FMassEntityHandle NewEntity;
            // NewEntity = MassEntitySubsystem->CreateEntity();
            
            ManagedEntities.Add(NewEntity);
        }
    }
    else
    {
        // Fallback: Create placeholder entities
        for (int32 i = 0; i < NumEntities; i++)
        {
            FMassEntityHandle PlaceholderEntity;
            ManagedEntities.Add(PlaceholderEntity);
        }
    }
    
    CurrentEntityCount = ManagedEntities.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Successfully spawned %d entities"), CurrentEntityCount);
}

void ACrowd_MassEntityManager::DestroyAllMassEntities()
{
    if (MassEntitySubsystem && ManagedEntities.Num() > 0)
    {
        // Destroy all managed entities
        for (const FMassEntityHandle& Entity : ManagedEntities)
        {
            // MassEntitySubsystem->DestroyEntity(Entity);
        }
    }
    
    ManagedEntities.Empty();
    CurrentEntityCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: All entities destroyed"));
}

void ACrowd_MassEntityManager::UpdateEntityBehaviors(const FCrowd_EntityBehaviorData& NewBehavior)
{
    DefaultBehavior = NewBehavior;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Updated entity behaviors - Flock Weight: %f, Avoidance Weight: %f"), 
           NewBehavior.FlockWeight, NewBehavior.AvoidanceWeight);
}

void ACrowd_MassEntityManager::UpdateLODDistances(float LOD1, float LOD2, float LOD3)
{
    EntityConfig.LODDistance1 = LOD1;
    EntityConfig.LODDistance2 = LOD2;
    EntityConfig.LODDistance3 = LOD3;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Updated LOD distances - LOD1: %f, LOD2: %f, LOD3: %f"), 
           LOD1, LOD2, LOD3);
}

int32 ACrowd_MassEntityManager::GetCurrentEntityCount() const
{
    return CurrentEntityCount;
}

void ACrowd_MassEntityManager::SetFlockingParameters(float FlockRadius, float AvoidRadius, float MoveSpeed)
{
    EntityConfig.FlockingRadius = FlockRadius;
    EntityConfig.AvoidanceRadius = AvoidRadius;
    EntityConfig.MovementSpeed = MoveSpeed;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Updated flocking parameters - Flock Radius: %f, Avoid Radius: %f, Speed: %f"), 
           FlockRadius, AvoidRadius, MoveSpeed);
}

void ACrowd_MassEntityManager::AddFlockingTarget(FVector TargetLocation, float Radius)
{
    DefaultBehavior.TargetLocation = TargetLocation;
    DefaultBehavior.WanderRadius = Radius;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Added flocking target at %s with radius %f"), 
           *TargetLocation.ToString(), Radius);
}

void ACrowd_MassEntityManager::RemoveFlockingTarget(FVector TargetLocation)
{
    // Reset to default wandering behavior
    DefaultBehavior.TargetLocation = FVector::ZeroVector;
    DefaultBehavior.SeekWeight = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Removed flocking target"));
}

void ACrowd_MassEntityManager::SetCrowdDensity(float DensityMultiplier)
{
    CrowdDensityMultiplier = FMath::Clamp(DensityMultiplier, 0.1f, 5.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Set crowd density multiplier to %f"), CrowdDensityMultiplier);
}

float ACrowd_MassEntityManager::GetCurrentCrowdDensity() const
{
    return CrowdDensityMultiplier;
}

float ACrowd_MassEntityManager::GetMassEntityPerformanceMetric() const
{
    // Calculate performance metric based on entity count and frame time
    if (LastFrameTime > 0.0f && CurrentEntityCount > 0)
    {
        float EntitiesPerMS = CurrentEntityCount / (LastFrameTime * 1000.0f);
        return EntitiesPerMS;
    }
    
    return 0.0f;
}

void ACrowd_MassEntityManager::OptimizeMassEntityPerformance()
{
    // Reduce entity count if performance is poor
    float PerformanceMetric = GetMassEntityPerformanceMetric();
    
    if (PerformanceMetric < 10.0f && CurrentEntityCount > 100)
    {
        int32 NewEntityCount = FMath::Max(100, CurrentEntityCount * 0.8f);
        
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Performance optimization - reducing entities from %d to %d"), 
               CurrentEntityCount, NewEntityCount);
        
        // Respawn with reduced count
        FVector CurrentLocation = GetActorLocation();
        DestroyAllMassEntities();
        SpawnMassEntities(NewEntityCount, CurrentLocation);
    }
}

void ACrowd_MassEntityManager::UpdateEntityLOD(float DeltaTime)
{
    // Update LOD based on distance to player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector ManagerLocation = GetActorLocation();
    float DistanceToPlayer = FVector::Dist(PlayerLocation, ManagerLocation);
    
    // Adjust entity count based on LOD distance
    int32 TargetEntityCount = EntityConfig.MaxEntities;
    
    if (DistanceToPlayer > EntityConfig.LODDistance3)
    {
        TargetEntityCount = EntityConfig.MaxEntities * 0.1f; // 10% entities at LOD3
    }
    else if (DistanceToPlayer > EntityConfig.LODDistance2)
    {
        TargetEntityCount = EntityConfig.MaxEntities * 0.3f; // 30% entities at LOD2
    }
    else if (DistanceToPlayer > EntityConfig.LODDistance1)
    {
        TargetEntityCount = EntityConfig.MaxEntities * 0.6f; // 60% entities at LOD1
    }
    
    // Apply density multiplier
    TargetEntityCount *= CrowdDensityMultiplier;
    
    // Adjust entity count if needed
    if (FMath::Abs(CurrentEntityCount - TargetEntityCount) > 50)
    {
        SpawnMassEntities(TargetEntityCount, ManagerLocation);
    }
}

void ACrowd_MassEntityManager::ProcessFlockingBehavior(float DeltaTime)
{
    if (!DefaultBehavior.bIsActive || ManagedEntities.Num() == 0)
    {
        return;
    }
    
    // Process flocking behavior for all entities
    for (const FMassEntityHandle& Entity : ManagedEntities)
    {
        // Calculate flocking forces
        FVector FlockingForce = CalculateFlockingForce(Entity);
        FVector AvoidanceForce = CalculateAvoidanceForce(Entity);
        FVector SeekForce = CalculateSeekForce(Entity, DefaultBehavior.TargetLocation);
        
        // Combine forces with weights
        FVector TotalForce = 
            FlockingForce * DefaultBehavior.FlockWeight +
            AvoidanceForce * DefaultBehavior.AvoidanceWeight +
            SeekForce * DefaultBehavior.SeekWeight;
        
        // Apply movement (placeholder - would use actual Mass Entity movement)
        // MassEntitySubsystem->SetEntityVelocity(Entity, TotalForce);
    }
}

void ACrowd_MassEntityManager::ValidateMassEntitySubsystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Re-validate subsystems
    if (!MassEntitySubsystem)
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    }
    
    if (!MassSpawnerSubsystem)
    {
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    }
    
    if (!MassSimulationSubsystem)
    {
        MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
    }
}

FVector ACrowd_MassEntityManager::CalculateFlockingForce(const FMassEntityHandle& Entity)
{
    // Placeholder flocking calculation
    FVector FlockingForce = FVector::ZeroVector;
    
    // In a real implementation, this would:
    // 1. Find nearby entities within flocking radius
    // 2. Calculate separation, alignment, and cohesion forces
    // 3. Return combined flocking force
    
    return FlockingForce;
}

FVector ACrowd_MassEntityManager::CalculateAvoidanceForce(const FMassEntityHandle& Entity)
{
    // Placeholder avoidance calculation
    FVector AvoidanceForce = FVector::ZeroVector;
    
    // In a real implementation, this would:
    // 1. Find nearby obstacles and entities within avoidance radius
    // 2. Calculate repulsion forces
    // 3. Return combined avoidance force
    
    return AvoidanceForce;
}

FVector ACrowd_MassEntityManager::CalculateSeekForce(const FMassEntityHandle& Entity, FVector Target)
{
    // Placeholder seek calculation
    FVector SeekForce = FVector::ZeroVector;
    
    if (Target != FVector::ZeroVector)
    {
        // In a real implementation, this would:
        // 1. Get entity current position
        // 2. Calculate direction to target
        // 3. Return seek force towards target
        
        SeekForce = Target.GetSafeNormal() * EntityConfig.MovementSpeed;
    }
    
    return SeekForce;
}