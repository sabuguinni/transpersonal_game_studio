// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
// FIXME: Missing header - #include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "MassEntityFragments.generated.h"

// Forward declarations
class UStaticMeshComponent;
class USkeletalMeshComponent;

/**
 * Fragment for dinosaur species identification and behavior parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdSim_MassDinosaurSpeciesFragment_05B : public FMassFragment
{
    GENERATED_BODY()

    // Species type enum
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpeciesID = 0;

    // Behavioral parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HerdInstinct = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCarnivore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPackHunter = false;

    // Size and physical attributes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BodySize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 2000.0f;
};

/**
 * Fragment for dinosaur daily routine and life cycle
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassDinosaurLifecycleFragment : public FMassFragment
{
    GENERATED_BODY()

    // Current activity state
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentActivity = 0; // 0=Idle, 1=Feeding, 2=Hunting, 3=Resting, 4=Migrating

    // Time tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivityStartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivityDuration = 300.0f; // 5 minutes default

    // Needs system
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThirstLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialNeed = 0.3f;

    // Last known locations for important resources
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastWaterSource = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastFoodSource = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TerritoryCenter = FVector::ZeroVector;
};

/**
 * Fragment for herd/pack behavior and social interactions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassHerdBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    // Herd identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = -1; // -1 means no herd

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHerdLeader = false;

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle HerdLeaderEntity;

    // Social behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentWeight = 1.0f;

    // Nearby entities tracking
// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> NearbyHerdMembers;

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> NearbyPredators;

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMassEntityHandle> NearbyPrey;

    // Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastCallTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AlertLevel = 0; // 0=Calm, 1=Cautious, 2=Alarmed, 3=Panicked
};

/**
 * Fragment for environmental awareness and navigation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassEnvironmentAwarenessFragment : public FMassFragment
{
    GENERATED_BODY()

    // Current biome and terrain preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PreferredBiome = 0; // 0=Forest, 1=Plains, 2=Swamp, 3=Desert

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerrainSuitability = 1.0f;

    // Weather and time awareness
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherComfort = 1.0f;

    // Navigation memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownWaterSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownFoodSources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownDangerZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> SafeRestingSpots;

    // Current navigation target
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CurrentDestination = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DestinationPriority = 0.0f;
};

/**
 * Fragment for visual representation and LOD management
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassVisualizationFragment : public FMassFragment
{
    GENERATED_BODY()

    // LOD system
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentLOD = 0; // 0=High, 1=Medium, 2=Low, 3=Impostor

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToPlayer = 10000.0f;

    // Visual representation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> HighDetailMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> MediumDetailMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> LowDetailMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> SpeciesMaterial;

    // Animation state
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AnimationState = 0; // 0=Idle, 1=Walk, 2=Run, 3=Feed, 4=Attack

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnimationSpeed = 1.0f;

    // Unique visual variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ColorVariation = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeVariation = 1.0f;
};

/**
 * Fragment for performance optimization and culling
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMassPerformanceFragment : public FMassFragment
{
    GENERATED_BODY()

    // Update frequency control
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UpdateInterval = 0.1f; // Seconds between updates

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastUpdateTime = 0.0f;

    // Importance scoring for priority updates
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ImportanceScore = 1.0f;

    // Culling flags
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInPlayerRange = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresHighFrequencyUpdate = false;

    // Performance budgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ProcessingBudget = 100; // Microseconds allocated per frame

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RenderingBudget = 50; // Draw calls allocated
};