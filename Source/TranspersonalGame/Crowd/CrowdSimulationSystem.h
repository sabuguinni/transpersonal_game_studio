#pragma once

#include "CoreMinimal.h"
#include "MassEntitySubsystem.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "CrowdSimulationSystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;
class UZoneGraphSubsystem;

/**
 * Dinosaur Herd Behavior Types
 * Defines the primary behavioral patterns for different dinosaur groups
 */
UENUM(BlueprintType)
enum class EDinosaurHerdBehavior : uint8
{
    Grazing,        // Peaceful herbivores feeding
    Migrating,      // Moving between feeding grounds
    Fleeing,        // Running from predators
    Hunting,        // Predator pack behavior
    Territorial,    // Defending territory
    Nesting,        // Protecting young/eggs
    Drinking        // Gathering at water sources
};

/**
 * Crowd Density Levels
 * Controls the intensity and scale of crowd simulation
 */
UENUM(BlueprintType)
enum class ECrowdDensity : uint8
{
    Sparse,         // 10-50 entities
    Medium,         // 100-500 entities
    Dense,          // 1000-5000 entities
    Massive         // 10000+ entities (migration events)
};

/**
 * Fragment: Dinosaur Herd Identity
 * Identifies which herd/pack this entity belongs to
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    int32 HerdID = -1;
    
    UPROPERTY()
    EDinosaurHerdBehavior CurrentBehavior = EDinosaurHerdBehavior::Grazing;
    
    UPROPERTY()
    FVector HerdCenter = FVector::ZeroVector;
    
    UPROPERTY()
    float HerdRadius = 1000.0f;
    
    UPROPERTY()
    int32 HerdSize = 0;
    
    UPROPERTY()
    bool bIsAlpha = false; // Leader of the herd
};

/**
 * Fragment: Dinosaur Species Data
 * Contains species-specific behavioral parameters
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    FName SpeciesName = NAME_None;
    
    UPROPERTY()
    bool bIsHerbivore = true;
    
    UPROPERTY()
    bool bIsPredator = false;
    
    UPROPERTY()
    float MaxSpeed = 800.0f; // cm/s
    
    UPROPERTY()
    float FleeDistance = 2000.0f; // Distance to start fleeing
    
    UPROPERTY()
    float AlertDistance = 1500.0f; // Distance to become alert
    
    UPROPERTY()
    float PreferredHerdSize = 15.0f;
    
    UPROPERTY()
    TArray<FName> PreySpecies; // What this species hunts
    
    UPROPERTY()
    TArray<FName> PredatorSpecies; // What hunts this species
};

/**
 * Fragment: Dinosaur Behavioral State
 * Tracks current behavioral state and transitions
 */
USTRUCT()
struct TRANSPERSONALGAME_API FDinosaurBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    EDinosaurHerdBehavior CurrentBehavior = EDinosaurHerdBehavior::Grazing;
    
    UPROPERTY()
    float BehaviorTimer = 0.0f;
    
    UPROPERTY()
    float BehaviorDuration = 60.0f; // How long to maintain current behavior
    
    UPROPERTY()
    FVector TargetLocation = FVector::ZeroVector;
    
    UPROPERTY()
    bool bHasThreatNearby = false;
    
    UPROPERTY()
    FMassEntityHandle ThreatEntity;
    
    UPROPERTY()
    float LastBehaviorChangeTime = 0.0f;
};

/**
 * Fragment: Crowd Avoidance Parameters
 * Controls how entities avoid each other and obstacles
 */
USTRUCT()
struct TRANSPERSONALGAME_API FCrowdAvoidanceFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY()
    float AvoidanceRadius = 200.0f;
    
    UPROPERTY()
    float SeparationWeight = 1.0f;
    
    UPROPERTY()
    float AlignmentWeight = 0.5f;
    
    UPROPERTY()
    float CohesionWeight = 0.3f;
    
    UPROPERTY()
    float ObstacleAvoidanceWeight = 2.0f;
    
    UPROPERTY()
    bool bUseRVO = true; // Reciprocal Velocity Obstacles
};

/**
 * Main Crowd Simulation System
 * Manages large-scale dinosaur herd behaviors using Mass Entity framework
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnDinosaurHerd(const FVector& Location, FName SpeciesName, int32 HerdSize, EDinosaurHerdBehavior InitialBehavior);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerMigrationEvent(const FVector& StartLocation, const FVector& EndLocation, float MigrationSpeed = 500.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetGlobalThreatLevel(float ThreatLevel); // 0.0 = peaceful, 1.0 = maximum danger
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterWaterSource(const FVector& Location, float Radius);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterFeedingGround(const FVector& Location, float Radius, FName PreferredSpecies);

    // Query Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetNearbyDinosaurCount(const FVector& Location, float Radius) const;
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FMassEntityHandle> GetNearbyDinosaurs(const FVector& Location, float Radius) const;
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    bool IsLocationSafeForPlayer(const FVector& Location, float SafetyRadius = 1000.0f) const;

protected:
    // Internal herd management
    void UpdateHerdCenters();
    void ProcessHerdBehaviorTransitions();
    void HandlePredatorPreyInteractions();
    void UpdateMigrationRoutes();
    
    // Crowd density management
    void OptimizeCrowdDensity(const FVector& PlayerLocation);
    void SpawnCrowdNearPlayer(const FVector& PlayerLocation, float SpawnRadius);
    void DespawnDistantCrowd(const FVector& PlayerLocation, float DespawnRadius);

private:
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem = nullptr;
    
    // Herd tracking
    TMap<int32, FVector> HerdCenters;
    TMap<int32, EDinosaurHerdBehavior> HerdBehaviors;
    TMap<int32, TArray<FMassEntityHandle>> HerdMembers;
    
    // Environmental data
    TArray<FVector> WaterSources;
    TArray<FVector> FeedingGrounds;
    TMap<FVector, FName> FeedingGroundSpecies;
    
    // Global state
    float GlobalThreatLevel = 0.0f;
    bool bMigrationEventActive = false;
    FVector MigrationStartPoint = FVector::ZeroVector;
    FVector MigrationEndPoint = FVector::ZeroVector;
    
    // Performance tracking
    int32 ActiveEntityCount = 0;
    int32 MaxEntityCount = 50000;
    
    // Herd ID counter
    int32 NextHerdID = 1;
};

/**
 * Processor: Dinosaur Herd Behavior
 * Handles the core behavioral logic for dinosaur herds
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurHerdBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurHerdBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};

/**
 * Processor: Crowd Avoidance
 * Implements flocking behavior and collision avoidance
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdAvoidanceProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowdAvoidanceProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    
    // Flocking calculations
    FVector CalculateSeparation(const FMassExecutionContext& Context, int32 EntityIndex, const TArrayView<FVector>& Locations);
    FVector CalculateAlignment(const FMassExecutionContext& Context, int32 EntityIndex, const TArrayView<FVector>& Velocities);
    FVector CalculateCohesion(const FMassExecutionContext& Context, int32 EntityIndex, const TArrayView<FVector>& Locations);
    FVector CalculateObstacleAvoidance(const FVector& Location, const FVector& Velocity);
};

/**
 * Processor: Migration Behavior
 * Handles large-scale migration events
 */
UCLASS()
class TRANSPERSONALGAME_API UMigrationBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMigrationBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};