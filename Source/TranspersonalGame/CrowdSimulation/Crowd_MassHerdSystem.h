#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassObserverProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassHerdSystem.generated.h"

// Forward declarations
struct FMassEntityHandle;
class UMassEntitySubsystem;

/**
 * Mass Entity processor for managing dinosaur herd behavior
 * Handles flocking, separation, cohesion, and alignment behaviors
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassHerdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassHerdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    // Herd behavior parameters
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float AlignmentRadius = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float AlignmentWeight = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float MaxForce = 100.0f;

    // Helper functions
    FVector CalculateSeparation(const FVector& EntityLocation, const TArray<FVector>& NeighborLocations) const;
    FVector CalculateCohesion(const FVector& EntityLocation, const TArray<FVector>& NeighborLocations) const;
    FVector CalculateAlignment(const FVector& EntityVelocity, const TArray<FVector>& NeighborVelocities) const;
    void FindNeighbors(const FVector& EntityLocation, const TArray<FMassEntityHandle>& AllEntities, 
                      const FMassEntityManager& EntityManager, float Radius, 
                      TArray<FVector>& OutLocations, TArray<FVector>& OutVelocities) const;
};

/**
 * Mass Entity processor for dinosaur predator-prey interactions
 * Handles hunting, fleeing, and territorial behaviors
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassPredatorPreyProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassPredatorPreyProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PredatorQuery;
    FMassEntityQuery PreyQuery;

    // Predator-prey parameters
    UPROPERTY(EditAnywhere, Category = "Predator Behavior")
    float HuntingRadius = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Predator Behavior")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Prey Behavior")
    float FleeRadius = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Prey Behavior")
    float PanicSpeed = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Territory")
    float TerritorialRadius = 1000.0f;

    // Helper functions
    FVector CalculateHuntingBehavior(const FVector& PredatorLocation, const FVector& PreyLocation) const;
    FVector CalculateFleeBehavior(const FVector& PreyLocation, const FVector& ThreatLocation) const;
    bool IsInTerritory(const FVector& EntityLocation, const FVector& TerritoryCenter, float Radius) const;
    void HandlePredatorPreyInteraction(FMassEntityHandle PredatorEntity, FMassEntityHandle PreyEntity,
                                     FMassEntityManager& EntityManager) const;
};

/**
 * Mass Entity processor for environmental awareness
 * Handles obstacle avoidance, terrain adaptation, and POI attraction
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassEnvironmentProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassEnvironmentProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    // Environment parameters
    UPROPERTY(EditAnywhere, Category = "Environment")
    float ObstacleAvoidanceRadius = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Environment")
    float WaterAttractionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Environment")
    float FoodAttractionRadius = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Environment")
    float ShelterAttractionRadius = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Environment")
    float TerrainAdaptationStrength = 2.0f;

    // POI locations cache
    UPROPERTY()
    TArray<FVector> WaterSources;

    UPROPERTY()
    TArray<FVector> FoodSources;

    UPROPERTY()
    TArray<FVector> ShelterLocations;

    // Helper functions
    FVector CalculateObstacleAvoidance(const FVector& EntityLocation, const FVector& EntityVelocity) const;
    FVector CalculatePOIAttraction(const FVector& EntityLocation, ECrowd_POIType POIType) const;
    FVector CalculateTerrainAdaptation(const FVector& EntityLocation) const;
    void UpdatePOICache();
    float GetTerrainHeight(const FVector& Location) const;
    FVector GetTerrainNormal(const FVector& Location) const;
};

/**
 * Mass Entity observer for crowd density management
 * Handles LOD transitions and performance optimization
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassDensityObserver : public UMassObserverProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassDensityObserver();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery HighDensityQuery;
    FMassEntityQuery MediumDensityQuery;
    FMassEntityQuery LowDensityQuery;

    // Density management parameters
    UPROPERTY(EditAnywhere, Category = "Density Management")
    float HighDensityThreshold = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Density Management")
    float MediumDensityThreshold = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Density Management")
    float DensityCheckRadius = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxEntitiesPerFrame = 1000;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float LODDistance2 = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float LODDistance3 = 4000.0f;

    // Helper functions
    float CalculateLocalDensity(const FVector& Location, const FMassEntityManager& EntityManager) const;
    ECrowd_LODLevel DetermineLODLevel(float DistanceToPlayer, float LocalDensity) const;
    void ApplyLODOptimizations(FMassEntityHandle Entity, ECrowd_LODLevel LODLevel, 
                              FMassEntityManager& EntityManager) const;
    FVector GetPlayerLocation() const;
};