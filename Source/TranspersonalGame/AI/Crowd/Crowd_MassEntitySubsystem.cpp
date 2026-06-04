#include "Crowd_MassEntitySubsystem.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

void UCrowd_MassEntitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    
    // Initialize default spawn config
    DefaultSpawnConfig.MaxEntities = 50000;
    DefaultSpawnConfig.SpawnRadius = 10000.0f;
    DefaultSpawnConfig.SpawnCenter = FVector::ZeroVector;
    DefaultSpawnConfig.AllowedBehaviors = {
        ECrowd_BehaviorState::Wandering,
        ECrowd_BehaviorState::Following,
        ECrowd_BehaviorState::Gathering
    };
    
    InitializeMassEntity();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Subsystem initialized - capacity: %d entities"), DefaultSpawnConfig.MaxEntities);
}

void UCrowd_MassEntitySubsystem::Deinitialize()
{
    ClearAllEntities();
    Super::Deinitialize();
}

void UCrowd_MassEntitySubsystem::InitializeMassEntity()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassEntitySubsystem not available"));
        return;
    }
    
    // Reserve entity capacity
    ActiveEntities.Reserve(DefaultSpawnConfig.MaxEntities);
    
    UE_LOG(LogTemp, Warning, TEXT("Mass Entity system initialized for crowd simulation"));
}

void UCrowd_MassEntitySubsystem::SpawnCrowdEntities(const FCrowd_SpawnConfig& Config)
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn entities - MassEntitySubsystem not available"));
        return;
    }
    
    // Clear existing entities first
    ClearAllEntities();
    
    // Spawn entities in batches for performance
    const int32 BatchSize = 1000;
    const int32 NumBatches = FMath::CeilToInt(static_cast<float>(Config.MaxEntities) / BatchSize);
    
    for (int32 BatchIndex = 0; BatchIndex < NumBatches; ++BatchIndex)
    {
        const int32 EntitiesInBatch = FMath::Min(BatchSize, Config.MaxEntities - (BatchIndex * BatchSize));
        
        for (int32 i = 0; i < EntitiesInBatch; ++i)
        {
            // Generate random spawn position within radius
            const float Angle = FMath::RandRange(0.0f, 2.0f * PI);
            const float Distance = FMath::RandRange(100.0f, Config.SpawnRadius);
            const FVector SpawnLocation = Config.SpawnCenter + FVector(
                FMath::Cos(Angle) * Distance,
                FMath::Sin(Angle) * Distance,
                0.0f
            );
            
            // Create entity
            FMassEntityHandle NewEntity = MassEntitySubsystem->CreateEntity();
            if (NewEntity.IsValid())
            {
                // Add transform fragment
                FTransformFragment TransformFragment;
                TransformFragment.GetMutableTransform().SetLocation(SpawnLocation);
                TransformFragment.GetMutableTransform().SetRotation(FQuat::Identity);
                
                // Add movement fragment
                FMassVelocityFragment VelocityFragment;
                VelocityFragment.Value = FVector::ZeroVector;
                
                ActiveEntities.Add(NewEntity);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd entities at location %s"), ActiveEntities.Num(), *Config.SpawnCenter.ToString());
}

void UCrowd_MassEntitySubsystem::UpdateCrowdBehavior(float DeltaTime)
{
    if (ActiveEntities.Num() == 0)
    {
        return;
    }
    
    ProcessMovement(DeltaTime);
    ProcessBehaviorStates(DeltaTime);
}

void UCrowd_MassEntitySubsystem::ProcessMovement(float DeltaTime)
{
    const float MaxSpeed = 200.0f;
    const float SeparationRadius = 100.0f;
    
    for (const FMassEntityHandle& Entity : ActiveEntities)
    {
        if (!Entity.IsValid())
        {
            continue;
        }
        
        // Get current location (simplified - in real implementation would use Mass fragments)
        FVector CurrentLocation = FVector::ZeroVector; // Would get from TransformFragment
        
        // Calculate flocking forces
        FVector FlockingForce = CalculateFlockingForce(Entity, CurrentLocation);
        
        // Apply movement towards destination
        FVector ToDestination = (CurrentDestination - CurrentLocation).GetSafeNormal();
        FVector FinalVelocity = (FlockingForce + ToDestination) * MaxSpeed * DeltaTime;
        
        // Update position (would update TransformFragment in real implementation)
        FVector NewLocation = CurrentLocation + FinalVelocity;
    }
}

void UCrowd_MassEntitySubsystem::ProcessBehaviorStates(float DeltaTime)
{
    // Process different behavior states
    for (const FMassEntityHandle& Entity : ActiveEntities)
    {
        if (!Entity.IsValid())
        {
            continue;
        }
        
        // Randomly change behavior states occasionally
        if (FMath::RandRange(0.0f, 1.0f) < 0.001f) // 0.1% chance per frame
        {
            // Switch to random allowed behavior
            if (DefaultSpawnConfig.AllowedBehaviors.Num() > 0)
            {
                const int32 RandomIndex = FMath::RandRange(0, DefaultSpawnConfig.AllowedBehaviors.Num() - 1);
                ECrowd_BehaviorState NewState = DefaultSpawnConfig.AllowedBehaviors[RandomIndex];
                // Would update behavior fragment here
            }
        }
    }
}

FVector UCrowd_MassEntitySubsystem::CalculateFlockingForce(const FMassEntityHandle& Entity, const FVector& CurrentLocation)
{
    FVector SeparationForce = FVector::ZeroVector;
    FVector AlignmentForce = FVector::ZeroVector;
    FVector CohesionForce = FVector::ZeroVector;
    
    const float SeparationRadius = 150.0f;
    const float AlignmentRadius = 300.0f;
    const float CohesionRadius = 500.0f;
    
    int32 SeparationCount = 0;
    int32 AlignmentCount = 0;
    int32 CohesionCount = 0;
    
    // Check neighbors (simplified - would use spatial partitioning in real implementation)
    for (const FMassEntityHandle& OtherEntity : ActiveEntities)
    {
        if (OtherEntity == Entity || !OtherEntity.IsValid())
        {
            continue;
        }
        
        FVector OtherLocation = FVector::ZeroVector; // Would get from fragment
        float Distance = FVector::Dist(CurrentLocation, OtherLocation);
        
        // Separation
        if (Distance < SeparationRadius && Distance > 0.0f)
        {
            FVector Diff = (CurrentLocation - OtherLocation).GetSafeNormal();
            SeparationForce += Diff / Distance; // Weight by inverse distance
            SeparationCount++;
        }
        
        // Alignment
        if (Distance < AlignmentRadius)
        {
            // Would get velocity from VelocityFragment
            FVector OtherVelocity = FVector::ForwardVector; // Placeholder
            AlignmentForce += OtherVelocity;
            AlignmentCount++;
        }
        
        // Cohesion
        if (Distance < CohesionRadius)
        {
            CohesionForce += OtherLocation;
            CohesionCount++;
        }
    }
    
    // Average and normalize forces
    if (SeparationCount > 0)
    {
        SeparationForce /= SeparationCount;
        SeparationForce = SeparationForce.GetSafeNormal() * 2.0f; // Separation weight
    }
    
    if (AlignmentCount > 0)
    {
        AlignmentForce /= AlignmentCount;
        AlignmentForce = AlignmentForce.GetSafeNormal() * 1.0f; // Alignment weight
    }
    
    if (CohesionCount > 0)
    {
        CohesionForce /= CohesionCount;
        CohesionForce = (CohesionForce - CurrentLocation).GetSafeNormal() * 1.5f; // Cohesion weight
    }
    
    return SeparationForce + AlignmentForce + CohesionForce;
}

void UCrowd_MassEntitySubsystem::SetCrowdDestination(FVector NewDestination)
{
    CurrentDestination = NewDestination;
    UE_LOG(LogTemp, Warning, TEXT("Crowd destination set to: %s"), *NewDestination.ToString());
}

int32 UCrowd_MassEntitySubsystem::GetActiveEntityCount() const
{
    return ActiveEntities.Num();
}

void UCrowd_MassEntitySubsystem::ClearAllEntities()
{
    if (MassEntitySubsystem)
    {
        for (const FMassEntityHandle& Entity : ActiveEntities)
        {
            if (Entity.IsValid())
            {
                MassEntitySubsystem->DestroyEntity(Entity);
            }
        }
    }
    
    ActiveEntities.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Cleared all crowd entities"));
}