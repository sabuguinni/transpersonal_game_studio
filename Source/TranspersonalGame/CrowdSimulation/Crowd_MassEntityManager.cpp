#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityTemplate.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    CurrentBehaviorState = ECrowd_MassBehaviorState::Idle;
    SpawnerSubsystem = nullptr;
    EntitySubsystem = nullptr;
    CrowdEntityTemplate = nullptr;
    
    // Performance tracking
    LastPerformanceCheck = 0.0f;
    ActiveEntityCount = 0;
    AverageFrameTime = 16.67f; // Target 60fps
    
    // Behavior state
    CrowdTargetLocation = FVector::ZeroVector;
    bHasCrowdTarget = false;
    
    // Panic system
    PanicSourceLocation = FVector::ZeroVector;
    PanicRadius = 0.0f;
    bInPanicMode = false;
    PanicStartTime = 0.0f;
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
    // Clean up all spawned entities
    DespawnAllEntities();
    
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update entity behaviors
    UpdateEntityBehaviors(DeltaTime);
    
    // Update LOD levels every 0.5 seconds
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= 0.5f)
    {
        UpdateLODLevels();
        LastPerformanceCheck = 0.0f;
    }
    
    // Handle panic mode timeout
    if (bInPanicMode)
    {
        float PanicDuration = GetWorld()->GetTimeSeconds() - PanicStartTime;
        if (PanicDuration > 30.0f) // 30 second panic duration
        {
            bInPanicMode = false;
            UpdateCrowdBehavior(ECrowd_MassBehaviorState::Wandering);
        }
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
    
    // Get Mass Entity subsystem
    EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!EntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Failed to get MassEntitySubsystem"));
        return;
    }
    
    // Get Mass Spawner subsystem
    SpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    if (!SpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Failed to get MassSpawnerSubsystem"));
        return;
    }
    
    // Create entity template
    CreateEntityTemplate();
    
    // Register processors
    RegisterMassProcessors();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Mass Entity system initialized successfully"));
}

void ACrowd_MassEntityManager::SpawnCrowdEntities(int32 EntityCount, FVector SpawnCenter, float SpawnRadius)
{
    if (!EntitySubsystem || !CrowdEntityTemplate)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassEntityManager: Cannot spawn entities - system not initialized"));
        return;
    }
    
    // Clamp entity count to configured maximum
    EntityCount = FMath::Min(EntityCount, EntityConfig.MaxEntities);
    
    // Clear existing entities
    DespawnAllEntities();
    
    // Spawn entities in a circle pattern
    for (int32 i = 0; i < EntityCount; i++)
    {
        // Calculate spawn position
        float Angle = (float)i / (float)EntityCount * 2.0f * PI;
        float Distance = FMath::RandRange(0.0f, SpawnRadius);
        FVector SpawnOffset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        FVector SpawnLocation = SpawnCenter + SpawnOffset;
        
        // Create entity
        FMassEntityHandle EntityHandle = EntitySubsystem->CreateEntity(CrowdEntityTemplate->GetArchetype());
        if (EntityHandle.IsValid())
        {
            // Set transform
            FTransform EntityTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector);
            EntitySubsystem->SetEntityFragmentData(EntityHandle, FTransformFragment(EntityTransform));
            
            // Set velocity
            FVector RandomVelocity = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                0.0f
            ).GetSafeNormal() * EntityConfig.MovementSpeed;
            EntitySubsystem->SetEntityFragmentData(EntityHandle, FMassVelocityFragment(RandomVelocity));
            
            SpawnedEntities.Add(EntityHandle);
        }
    }
    
    ActiveEntityCount = SpawnedEntities.Num();
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Spawned %d entities at %s"), ActiveEntityCount, *SpawnCenter.ToString());
}

void ACrowd_MassEntityManager::DespawnAllEntities()
{
    if (!EntitySubsystem)
    {
        return;
    }
    
    // Destroy all spawned entities
    for (FMassEntityHandle EntityHandle : SpawnedEntities)
    {
        if (EntityHandle.IsValid())
        {
            EntitySubsystem->DestroyEntity(EntityHandle);
        }
    }
    
    SpawnedEntities.Empty();
    ActiveEntityCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Despawned all entities"));
}

void ACrowd_MassEntityManager::UpdateCrowdBehavior(ECrowd_MassBehaviorState NewBehaviorState)
{
    CurrentBehaviorState = NewBehaviorState;
    
    // Update entity behaviors based on new state
    if (!EntitySubsystem)
    {
        return;
    }
    
    for (FMassEntityHandle EntityHandle : SpawnedEntities)
    {
        if (!EntityHandle.IsValid())
        {
            continue;
        }
        
        FVector NewVelocity = FVector::ZeroVector;
        
        switch (CurrentBehaviorState)
        {
            case ECrowd_MassBehaviorState::Idle:
                NewVelocity = FVector::ZeroVector;
                break;
                
            case ECrowd_MassBehaviorState::Wandering:
                NewVelocity = FVector(
                    FMath::RandRange(-1.0f, 1.0f),
                    FMath::RandRange(-1.0f, 1.0f),
                    0.0f
                ).GetSafeNormal() * EntityConfig.MovementSpeed * 0.5f;
                break;
                
            case ECrowd_MassBehaviorState::Fleeing:
                if (bInPanicMode)
                {
                    // Get entity position
                    FTransformFragment* TransformFragment = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle);
                    if (TransformFragment)
                    {
                        FVector FleeDirection = (TransformFragment->GetTransform().GetLocation() - PanicSourceLocation).GetSafeNormal();
                        NewVelocity = FleeDirection * EntityConfig.MovementSpeed * 2.0f; // Double speed when fleeing
                    }
                }
                break;
                
            case ECrowd_MassBehaviorState::Gathering:
                if (bHasCrowdTarget)
                {
                    // Get entity position
                    FTransformFragment* TransformFragment = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle);
                    if (TransformFragment)
                    {
                        FVector GatherDirection = (CrowdTargetLocation - TransformFragment->GetTransform().GetLocation()).GetSafeNormal();
                        NewVelocity = GatherDirection * EntityConfig.MovementSpeed;
                    }
                }
                break;
                
            case ECrowd_MassBehaviorState::Following:
                // Follow player or designated target
                if (bHasCrowdTarget)
                {
                    FTransformFragment* TransformFragment = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle);
                    if (TransformFragment)
                    {
                        FVector FollowDirection = (CrowdTargetLocation - TransformFragment->GetTransform().GetLocation()).GetSafeNormal();
                        NewVelocity = FollowDirection * EntityConfig.MovementSpeed * 0.8f;
                    }
                }
                break;
                
            case ECrowd_MassBehaviorState::Panicking:
                // Random high-speed movement
                NewVelocity = FVector(
                    FMath::RandRange(-1.0f, 1.0f),
                    FMath::RandRange(-1.0f, 1.0f),
                    0.0f
                ).GetSafeNormal() * EntityConfig.MovementSpeed * 3.0f;
                break;
        }
        
        // Apply new velocity
        EntitySubsystem->SetEntityFragmentData(EntityHandle, FMassVelocityFragment(NewVelocity));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Updated behavior to %d for %d entities"), 
        (int32)CurrentBehaviorState, SpawnedEntities.Num());
}

void ACrowd_MassEntityManager::SetCrowdTarget(FVector TargetLocation)
{
    CrowdTargetLocation = TargetLocation;
    bHasCrowdTarget = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Set crowd target to %s"), *TargetLocation.ToString());
}

void ACrowd_MassEntityManager::TriggerCrowdPanic(FVector PanicSource, float PanicRadius)
{
    PanicSourceLocation = PanicSource;
    this->PanicRadius = PanicRadius;
    bInPanicMode = true;
    PanicStartTime = GetWorld()->GetTimeSeconds();
    
    // Switch to fleeing behavior
    UpdateCrowdBehavior(ECrowd_MassBehaviorState::Fleeing);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Triggered panic at %s with radius %f"), 
        *PanicSource.ToString(), PanicRadius);
}

void ACrowd_MassEntityManager::UpdateLODLevels()
{
    if (!EntitySubsystem)
    {
        return;
    }
    
    // Get player location for LOD calculations
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    // Update LOD for each entity based on distance to player
    for (FMassEntityHandle EntityHandle : SpawnedEntities)
    {
        if (!EntityHandle.IsValid())
        {
            continue;
        }
        
        FTransformFragment* TransformFragment = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle);
        if (TransformFragment)
        {
            float DistanceToPlayer = FVector::Dist(TransformFragment->GetTransform().GetLocation(), PlayerLocation);
            
            // Simple LOD system: close, medium, far
            int32 LODLevel = 0;
            if (DistanceToPlayer > EntityConfig.LODDistance * 2.0f)
            {
                LODLevel = 2; // Far - minimal representation
            }
            else if (DistanceToPlayer > EntityConfig.LODDistance)
            {
                LODLevel = 1; // Medium - reduced detail
            }
            else
            {
                LODLevel = 0; // Close - full detail
            }
            
            // Apply LOD (in a real implementation, this would affect rendering)
            // For now, just log the LOD changes for very distant entities
            if (LODLevel == 2 && FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance to log
            {
                UE_LOG(LogTemp, Log, TEXT("Entity at distance %f set to LOD %d"), DistanceToPlayer, LODLevel);
            }
        }
    }
}

int32 ACrowd_MassEntityManager::GetActiveEntityCount() const
{
    return ActiveEntityCount;
}

float ACrowd_MassEntityManager::GetCurrentPerformanceMetric() const
{
    return AverageFrameTime;
}

void ACrowd_MassEntityManager::CreateEntityTemplate()
{
    // In a full implementation, this would create a proper UMassEntityTemplate
    // For now, we'll create a basic template programmatically
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Creating entity template"));
    
    // This is a simplified version - in practice, you'd set up fragments for:
    // - Transform (position, rotation, scale)
    // - Velocity (movement)
    // - Representation (visual mesh)
    // - LOD (level of detail)
    // - Behavior state
}

void ACrowd_MassEntityManager::RegisterMassProcessors()
{
    // In a full implementation, this would register custom Mass processors for:
    // - Movement processing
    // - Behavior processing
    // - LOD processing
    // - Collision avoidance
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Registering Mass processors"));
}

void ACrowd_MassEntityManager::UpdateEntityBehaviors(float DeltaTime)
{
    // Update average frame time for performance tracking
    AverageFrameTime = (AverageFrameTime * 0.9f) + (DeltaTime * 1000.0f * 0.1f);
    
    // Handle entity culling if performance is poor
    if (AverageFrameTime > 20.0f && ActiveEntityCount > 1000) // If frame time > 20ms and many entities
    {
        HandleEntityCulling();
    }
}

void ACrowd_MassEntityManager::HandleEntityCulling()
{
    // Cull distant entities to maintain performance
    if (!EntitySubsystem)
    {
        return;
    }
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    float CullDistance = EntityConfig.LODDistance * 3.0f;
    int32 CulledCount = 0;
    
    for (int32 i = SpawnedEntities.Num() - 1; i >= 0; i--)
    {
        FMassEntityHandle EntityHandle = SpawnedEntities[i];
        if (!EntityHandle.IsValid())
        {
            SpawnedEntities.RemoveAt(i);
            continue;
        }
        
        FTransformFragment* TransformFragment = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(EntityHandle);
        if (TransformFragment)
        {
            float DistanceToPlayer = FVector::Dist(TransformFragment->GetTransform().GetLocation(), PlayerLocation);
            
            if (DistanceToPlayer > CullDistance)
            {
                EntitySubsystem->DestroyEntity(EntityHandle);
                SpawnedEntities.RemoveAt(i);
                CulledCount++;
            }
        }
    }
    
    ActiveEntityCount = SpawnedEntities.Num();
    
    if (CulledCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Culled %d distant entities for performance"), CulledCount);
    }
}