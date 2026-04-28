// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassExecutionContext.h"
#include "MassEntityFragments.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
// FIXME: Missing header - #include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "MassEntityProcessors.generated.h"

/**
 * Processor for managing dinosaur lifecycle and daily routines
 */
UCLASS()
class TRANSPERSONALGAME_API UMassDinosaurLifecycleProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassDinosaurLifecycleProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    // Activity transition logic
    void ProcessActivityTransitions(FMassExecutionContext& Context);
    void UpdateNeeds(FMassExecutionContext& Context);
    void FindNearbyResources(FMassExecutionContext& Context);

    // Activity-specific behaviors
    void ProcessIdleBehavior(FMassDinosaurLifecycleFragment& Lifecycle, FMassFragment& Species, 
                           FTransformFragment& Transform, float DeltaTime);
    void ProcessFeedingBehavior(FMassDinosaurLifecycleFragment& Lifecycle, FMassFragment& Species, 
                              FTransformFragment& Transform, float DeltaTime);
    void ProcessHuntingBehavior(FMassDinosaurLifecycleFragment& Lifecycle, FMassFragment& Species, 
                              FTransformFragment& Transform, float DeltaTime);
    void ProcessRestingBehavior(FMassDinosaurLifecycleFragment& Lifecycle, FMassFragment& Species, 
                              FTransformFragment& Transform, float DeltaTime);
    void ProcessMigratingBehavior(FMassDinosaurLifecycleFragment& Lifecycle, FMassFragment& Species, 
                                FTransformFragment& Transform, float DeltaTime);

public:
    // Configuration parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifecycle")
    float HungerDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifecycle")
    float ThirstDecayRate = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifecycle")
    float EnergyDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lifecycle")
    float ResourceSearchRadius = 2000.0f;
};

/**
 * Processor for herd behavior and social interactions
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSim_MassHerdBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowdSim_MassHerdBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery HerdMemberQuery;
    FMassEntityQuery HerdLeaderQuery;

    // Herd management
    void UpdateHerdMembership(FMassExecutionContext& Context);
    void ProcessHerdMovement(FMassExecutionContext& Context);
    void HandleHerdCommunication(FMassExecutionContext& Context);

    // Flocking behaviors
    FVector CalculateCohesion(const FMassHerdBehaviorFragment& HerdBehavior, 
                            const FTransformFragment& Transform, 
                            const TArray<FTransformFragment>& NearbyTransforms);
    FVector CalculateSeparation(const FMassHerdBehaviorFragment& HerdBehavior, 
                              const FTransformFragment& Transform, 
                              const TArray<FTransformFragment>& NearbyTransforms);
    FVector CalculateAlignment(const FMassHerdBehaviorFragment& HerdBehavior, 
                             const FMassVelocityFragment& Velocity, 
                             const TArray<FMassVelocityFragment>& NearbyVelocities);

    // Threat response
    void ProcessThreatResponse(FMassExecutionContext& Context);
    void UpdateAlertLevels(FMassExecutionContext& Context);

public:
    // Configuration parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float HerdFormationRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float MaxHerdSize = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float LeadershipChangeThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Behavior")
    float CommunicationRange = 3000.0f;
};

/**
 * Processor for environmental awareness and navigation
 */
UCLASS()
class TRANSPERSONALGAME_API UMassEnvironmentProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassEnvironmentProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EnvironmentQuery;

    // Environmental assessment
    void UpdateBiomePreferences(FMassExecutionContext& Context);
    void UpdateWeatherComfort(FMassExecutionContext& Context);
    void UpdateTimeOfDayBehavior(FMassExecutionContext& Context);

    // Navigation and pathfinding
    void UpdateNavigationTargets(FMassExecutionContext& Context);
    void ProcessDestinationSelection(FMassExecutionContext& Context);
    void UpdateMemoryMaps(FMassExecutionContext& Context);

    // Resource detection
    void ScanForWaterSources(FMassExecutionContext& Context);
    void ScanForFoodSources(FMassExecutionContext& Context);
    void IdentifyDangerZones(FMassExecutionContext& Context);

public:
    // Configuration parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float EnvironmentScanRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float MemoryRetentionTime = 3600.0f; // 1 hour

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float BiomeTransitionSpeed = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    int32 MaxMemoryLocations = 50;
};

/**
 * Processor for visual representation and LOD management
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSim_MassVisualizationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowdSim_MassVisualizationProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery VisualizationQuery;

    // LOD management
    void UpdateLODLevels(FMassExecutionContext& Context);
    void CalculateDistanceToPlayer(FMassExecutionContext& Context);
    void OptimizeRenderingBudget(FMassExecutionContext& Context);

    // Visual updates
    void UpdateAnimationStates(FMassExecutionContext& Context);
    void UpdateMeshRepresentation(FMassExecutionContext& Context);
    void UpdateMaterialProperties(FMassExecutionContext& Context);

    // Culling and visibility
    void ProcessVisibilityCulling(FMassExecutionContext& Context);
    void UpdateImportanceScoring(FMassExecutionContext& Context);

public:
    // LOD distance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    float HighDetailDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    float MediumDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    float LowDetailDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    float CullingDistance = 15000.0f;

    // Performance budgets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxHighDetailEntities = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMediumDetailEntities = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxLowDetailEntities = 2000;
};

/**
 * Processor for performance optimization and load balancing
 */
UCLASS()
class TRANSPERSONALGAME_API UMassPerformanceOptimizationProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassPerformanceOptimizationProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PerformanceQuery;

    // Performance monitoring
    void MonitorFrameTime(FMassExecutionContext& Context);
    void UpdateProcessingBudgets(FMassExecutionContext& Context);
    void BalanceUpdateFrequencies(FMassExecutionContext& Context);

    // Adaptive quality scaling
    void AdjustSimulationQuality(FMassExecutionContext& Context);
    void OptimizeEntityCounts(FMassExecutionContext& Context);
    void ManageMemoryUsage(FMassExecutionContext& Context);

    // Priority scheduling
    void UpdateImportanceScores(FMassExecutionContext& Context);
    void ScheduleHighPriorityUpdates(FMassExecutionContext& Context);
    void DeferLowPriorityUpdates(FMassExecutionContext& Context);

public:
    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTime = 33.33f; // 30 FPS minimum

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEntities = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceScalingFactor = 0.8f;

    // Memory management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryUsageMB = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryCleanupThreshold = 0.8f;
};