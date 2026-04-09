// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassAvoidanceFragments.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "CrowdSimulationSystem.h"
#include "MassCrowdProcessor.generated.h"

/**
 * Mass Fragment for crowd behavior data
 */
USTRUCT()
struct TRANSPERSONALGAME_API FMassCrowdBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    // Current behavior state
    ECrowdBehaviorType CurrentBehavior = ECrowdBehaviorType::Wanderer;
    
    // Group information
    int32 GroupID = -1;
    bool bIsGroupLeader = false;
    int32 GroupSize = 1;
    
    // Behavior timing
    float BehaviorTimer = 0.0f;
    float BehaviorDuration = 30.0f; // How long to maintain current behavior
    
    // Formation data
    ECrowdFormation CurrentFormation = ECrowdFormation::Scattered;
    FVector FormationOffset = FVector::ZeroVector;
    
    // Response state
    ECrowdResponse LastResponse = ECrowdResponse::IgnoreStimulus;
    float ResponseCooldown = 0.0f;
    
    // Stress and panic levels
    float StressLevel = 0.0f;
    float PanicThreshold = 0.8f;
    bool bInPanic = false;
};

/**
 * Mass Fragment for crowd entity configuration
 */
USTRUCT()
struct TRANSPERSONALGAME_API FMassCrowdConfigFragment : public FMassFragment
{
    GENERATED_BODY()

    // Entity configuration reference
    FCrowdEntityConfig EntityConfig;
    
    // Runtime modifiers
    float SpeedMultiplier = 1.0f;
    float AggressionLevel = 0.5f;
    float CuriosityLevel = 0.5f;
    float SocialLevel = 0.5f;
};

/**
 * Mass Fragment for crowd perception data
 */
USTRUCT()
struct TRANSPERSONALGAME_API FMassCrowdPerceptionFragment : public FMassFragment
{
    GENERATED_BODY()

    // Detected threats
    TArray<FMassEntityHandle> DetectedThreats;
    TArray<FMassEntityHandle> DetectedAllies;
    TArray<FMassEntityHandle> DetectedFood;
    
    // Environmental awareness
    FVector LastKnownThreatLocation = FVector::ZeroVector;
    float TimeSinceLastThreatSeen = 0.0f;
    
    // Social awareness
    FVector GroupCenter = FVector::ZeroVector;
    float DistanceToGroupCenter = 0.0f;
    int32 NearbyGroupMembers = 0;
};

/**
 * Mass Processor for crowd behavior logic
 */
UCLASS()
class TRANSPERSONALGAME_API UMassCrowdBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassCrowdBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query for entities with crowd behavior
    FMassEntityQuery EntityQuery;
    
    // Behavior processing methods
    void ProcessGrazingHerd(FMassEntityManager& EntityManager, FMassExecutionContext& Context, 
                           const FMassEntityHandle& Entity, FMassCrowdBehaviorFragment& Behavior, 
                           const FTransformFragment& Transform, FMassVelocityFragment& Velocity);
                           
    void ProcessHuntingPack(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
                           const FMassEntityHandle& Entity, FMassCrowdBehaviorFragment& Behavior,
                           const FTransformFragment& Transform, FMassVelocityFragment& Velocity);
                           
    void ProcessFleeingHerd(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
                           const FMassEntityHandle& Entity, FMassCrowdBehaviorFragment& Behavior,
                           const FTransformFragment& Transform, FMassVelocityFragment& Velocity);
                           
    void ProcessDefensiveCircle(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
                               const FMassEntityHandle& Entity, FMassCrowdBehaviorFragment& Behavior,
                               const FTransformFragment& Transform, FMassVelocityFragment& Velocity);

    // Formation processing
    void ApplyFormation(const ECrowdFormation Formation, const FVector& GroupCenter, 
                       const int32 GroupSize, const int32 EntityIndex, FVector& OutTargetPosition);
                       
    // Group management
    void UpdateGroupCohesion(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
                            const FMassEntityHandle& Entity, FMassCrowdBehaviorFragment& Behavior,
                            const FTransformFragment& Transform);
                            
    // Stress and panic system
    void UpdateStressLevel(FMassCrowdBehaviorFragment& Behavior, const FMassCrowdPerceptionFragment& Perception, float DeltaTime);
    
    // Behavior transition logic
    bool ShouldChangeBehavior(const FMassCrowdBehaviorFragment& Behavior, const FMassCrowdConfigFragment& Config, 
                             const FMassCrowdPerceptionFragment& Perception);
                             
    ECrowdBehaviorType SelectNewBehavior(const FMassCrowdBehaviorFragment& CurrentBehavior, 
                                        const FMassCrowdConfigFragment& Config,
                                        const FMassCrowdPerceptionFragment& Perception);
};

/**
 * Mass Processor for crowd perception and awareness
 */
UCLASS()
class TRANSPERSONALGAME_API UMassCrowdPerceptionProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassCrowdPerceptionProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    
    // Perception processing
    void UpdatePerception(FMassEntityManager& EntityManager, const FMassEntityHandle& Entity,
                         FMassCrowdPerceptionFragment& Perception, const FTransformFragment& Transform,
                         const FMassCrowdConfigFragment& Config, float DeltaTime);
                         
    // Threat detection
    void DetectThreats(FMassEntityManager& EntityManager, const FMassEntityHandle& Entity,
                      FMassCrowdPerceptionFragment& Perception, const FTransformFragment& Transform,
                      const FMassCrowdConfigFragment& Config);
                      
    // Social awareness
    void UpdateSocialAwareness(FMassEntityManager& EntityManager, const FMassEntityHandle& Entity,
                              FMassCrowdPerceptionFragment& Perception, const FTransformFragment& Transform,
                              const FMassCrowdBehaviorFragment& Behavior);
                              
    // Group center calculation
    FVector CalculateGroupCenter(FMassEntityManager& EntityManager, const FMassCrowdBehaviorFragment& Behavior,
                                const FTransformFragment& Transform);
};

/**
 * Mass Processor for crowd movement and navigation
 */
UCLASS()
class TRANSPERSONALGAME_API UMassCrowdMovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassCrowdMovementProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    
    // Movement processing
    void ProcessMovement(FMassEntityManager& EntityManager, const FMassEntityHandle& Entity,
                        const FMassCrowdBehaviorFragment& Behavior, const FMassCrowdConfigFragment& Config,
                        const FTransformFragment& Transform, FMassVelocityFragment& Velocity,
                        FMassMoveTargetFragment& MoveTarget, float DeltaTime);
                        
    // Behavior-specific movement
    FVector CalculateGrazingMovement(const FTransformFragment& Transform, const FMassCrowdBehaviorFragment& Behavior);
    FVector CalculateHuntingMovement(const FTransformFragment& Transform, const FMassCrowdBehaviorFragment& Behavior,
                                    const FMassCrowdPerceptionFragment& Perception);
    FVector CalculateFleeingMovement(const FTransformFragment& Transform, const FMassCrowdBehaviorFragment& Behavior,
                                    const FMassCrowdPerceptionFragment& Perception);
                                    
    // Formation movement
    FVector CalculateFormationMovement(const FTransformFragment& Transform, const FMassCrowdBehaviorFragment& Behavior,
                                      const FMassCrowdPerceptionFragment& Perception);
                                      
    // Avoidance and steering
    FVector CalculateAvoidanceForce(FMassEntityManager& EntityManager, const FMassEntityHandle& Entity,
                                   const FTransformFragment& Transform, const FMassCrowdConfigFragment& Config);
                                   
    // Speed calculation based on behavior and stress
    float CalculateMovementSpeed(const FMassCrowdBehaviorFragment& Behavior, const FMassCrowdConfigFragment& Config);
};

/**
 * Mass Processor for crowd spawning and despawning
 */
UCLASS()
class TRANSPERSONALGAME_API UMassCrowdSpawnProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassCrowdSpawnProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    
    // Spawning logic
    void ProcessSpawning(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    // Group spawning
    void SpawnGroup(FMassEntityManager& EntityManager, const FCrowdEntityConfig& Config,
                   const FVector& SpawnLocation, int32 GroupSize, ECrowdBehaviorType Behavior);
                   
    // Despawning logic
    void ProcessDespawning(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    // Distance-based culling
    bool ShouldDespawnEntity(const FTransformFragment& Transform, const FVector& PlayerLocation, float CullDistance);
};