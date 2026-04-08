#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "HerdBehaviorFragment.generated.h"

/**
 * Fragment for herd behavior data in Mass Entity system
 * Contains all data needed for flocking/herding behavior
 */
USTRUCT()
struct TRANSPERSONALGAME_API FHerdBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    // Herd identification
    UPROPERTY()
    int32 HerdID = -1;

    UPROPERTY()
    bool bIsHerdLeader = false;

    // Behavior weights
    UPROPERTY()
    float CohesionWeight = 1.0f;

    UPROPERTY()
    float SeparationWeight = 2.0f;

    UPROPERTY()
    float AlignmentWeight = 1.0f;

    UPROPERTY()
    float LeaderFollowWeight = 3.0f;

    // Behavior radii
    UPROPERTY()
    float CohesionRadius = 500.0f;

    UPROPERTY()
    float SeparationRadius = 100.0f;

    UPROPERTY()
    float AlignmentRadius = 300.0f;

    UPROPERTY()
    float LeaderFollowRadius = 800.0f;

    // Current behavior state
    UPROPERTY()
    FVector DesiredDirection = FVector::ZeroVector;

    UPROPERTY()
    float DesiredSpeed = 200.0f;

    UPROPERTY()
    float MaxSpeed = 400.0f;

    UPROPERTY()
    float MinSpeed = 50.0f;

    // Panic/fear system
    UPROPERTY()
    bool bIsPanicked = false;

    UPROPERTY()
    float PanicLevel = 0.0f; // 0.0 to 1.0

    UPROPERTY()
    float PanicDecayRate = 0.5f; // Per second

    UPROPERTY()
    FVector ThreatDirection = FVector::ZeroVector;

    UPROPERTY()
    float LastThreatTime = 0.0f;

    // Migration behavior
    UPROPERTY()
    bool bIsOnMigration = false;

    UPROPERTY()
    int32 CurrentMigrationWaypoint = 0;

    UPROPERTY()
    float MigrationProgress = 0.0f; // 0.0 to 1.0

    // Feeding behavior
    UPROPERTY()
    bool bIsFeeding = false;

    UPROPERTY()
    float FeedingTime = 0.0f;

    UPROPERTY()
    float MaxFeedingTime = 30.0f; // 30 seconds

    UPROPERTY()
    float HungerLevel = 0.5f; // 0.0 to 1.0

    // Rest behavior
    UPROPERTY()
    bool bIsResting = false;

    UPROPERTY()
    float RestTime = 0.0f;

    UPROPERTY()
    float MaxRestTime = 60.0f; // 1 minute

    UPROPERTY()
    float EnergyLevel = 1.0f; // 0.0 to 1.0

    // Social hierarchy
    UPROPERTY()
    float DominanceLevel = 0.5f; // 0.0 to 1.0

    UPROPERTY()
    int32 SocialRank = 0; // 0 = lowest, higher = more dominant

    // Age and size factors
    UPROPERTY()
    float AgeCategory = 0.5f; // 0.0 = juvenile, 1.0 = elder

    UPROPERTY()
    float SizeMultiplier = 1.0f; // Affects movement speed and behavior

    // Environmental awareness
    UPROPERTY()
    float WaterNeed = 0.0f; // 0.0 to 1.0

    UPROPERTY()
    float DistanceToWater = 10000.0f;

    UPROPERTY()
    FVector NearestWaterLocation = FVector::ZeroVector;

    UPROPERTY()
    bool bCanSeeWater = false;

    // Seasonal behavior modifiers
    UPROPERTY()
    float SeasonalMigrationUrge = 0.0f; // 0.0 to 1.0

    UPROPERTY()
    float BreedingUrge = 0.0f; // 0.0 to 1.0

    UPROPERTY()
    bool bIsMatingPair = false;

    UPROPERTY()
    FMassEntityHandle MateEntity;

    // Debug information
    UPROPERTY()
    FString CurrentBehaviorState = TEXT("Grazing");

    UPROPERTY()
    float LastBehaviorChangeTime = 0.0f;
};