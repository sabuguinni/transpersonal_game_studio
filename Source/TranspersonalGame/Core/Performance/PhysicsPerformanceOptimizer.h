// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Chaos/ChaosEngineInterface.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "PhysicsPerformanceOptimizer.generated.h"

/** Physics object types for performance management */
UENUM(BlueprintType)
enum class EPhysicsObjectType : uint8
{
    CreatureRagdolls    UMETA(DisplayName = "Creature Ragdolls"),
    DestructionDebris   UMETA(DisplayName = "Destruction Debris"),
    EnvironmentalProps  UMETA(DisplayName = "Environmental Props"),
    ProjectilePhysics   UMETA(DisplayName = "Projectile Physics"),
    VehiclePhysics      UMETA(DisplayName = "Vehicle Physics")
};

/** Physics LOD levels */
UENUM(BlueprintType)
enum class EPhysicsLODLevel : uint8
{
    LOD0_Full       UMETA(DisplayName = "LOD0 - Full Physics"),
    LOD1_Reduced    UMETA(DisplayName = "LOD1 - Reduced Physics"),
    LOD2_Basic      UMETA(DisplayName = "LOD2 - Basic Physics"),
    LOD3_Kinematic  UMETA(DisplayName = "LOD3 - Kinematic Only")
};

/** Physics performance metrics structure */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPhysicsPerformanceMetrics
{
    GENERATED_BODY()

    /** Physics simulation time in milliseconds */
    UPROPERTY(BlueprintReadOnly)
    float PhysicsSimulationTimeMS = 0.0f;

    /** Number of active physics bodies */
    UPROPERTY(BlueprintReadOnly)
    int32 ActivePhysicsBodies = 0;

    /** Number of active ragdolls */
    UPROPERTY(BlueprintReadOnly)
    int32 ActiveRagdolls = 0;

    /** Number of destruction debris pieces */
    UPROPERTY(BlueprintReadOnly)
    int32 DestructionDebrisCount = 0;

    /** Current physics substep count */
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentSubsteps = 6;

    /** Number of collision checks per frame */
    UPROPERTY(BlueprintReadOnly)
    int32 CollisionChecksPerFrame = 0;

    /** Physics memory usage in MB */
    UPROPERTY(BlueprintReadOnly)
    float PhysicsMemoryUsageMB = 0.0f;

    /** Physics CPU usage percentage */
    UPROPERTY(BlueprintReadOnly)
    float PhysicsCPUUsagePercent = 0.0f;

    /** Whether physics is within performance budget */
    UPROPERTY(BlueprintReadOnly)
    bool bWithinPerformanceBudget = true;
};

/**
 * @brief Physics Performance Optimizer for Jurassic Survival Game
 * 
 * Specialized optimizer for physics systems implemented by Core Systems Programmer.
 * Ensures physics simulation runs at target framerates while maintaining visual quality.
 * 
 * Key Optimizations:
 * - Dynamic LOD for physics collision meshes
 * - Intelligent culling of distant physics objects
 * - Adaptive physics substep scaling
 * - Smart ragdoll management (max concurrent ragdolls)
 * - Destruction debris cleanup and pooling
 * - Distance-based physics complexity reduction
 * 
 * Performance Philosophy:
 * "Performance is not about removing features — it's about delivering them intelligently"
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Initialize physics performance optimization
     * 
     * Sets up performance monitoring for all physics systems:
     * - Collision detection optimization
     * - Ragdoll management
     * - Destruction system optimization
     * - Dynamic physics LOD system
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void InitializePhysicsOptimization();

    /**
     * @brief Optimize collision detection performance
     * 
     * Implements distance-based collision LOD:
     * - Full collision for objects within 50m
     * - Simplified collision for objects 50-200m
     * - Basic bounds collision for objects >200m
     * - No collision for objects >500m
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizeCollisionDetection();

    /**
     * @brief Manage ragdoll physics performance
     * 
     * Intelligent ragdoll management:
     * - Maximum 8 concurrent ragdolls (console) / 16 (PC)
     * - Automatic cleanup of distant ragdolls
     * - Simplified ragdoll physics for background creatures
     * - Ragdoll pooling to avoid allocation overhead
     * 
     * @param MaxConcurrentRagdolls Maximum number of active ragdolls
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ManageRagdollPerformance(int32 MaxConcurrentRagdolls = 8);

    /**
     * @brief Optimize destruction system performance
     * 
     * Smart destruction management:
     * - Limit destruction debris count
     * - Automatic debris cleanup after time/distance
     * - Simplified destruction for distant objects
     * - Destruction effect pooling
     * 
     * @param MaxDebrisCount Maximum destruction debris pieces
     * @param DebrisLifetime How long debris stays active (seconds)
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizeDestructionSystem(int32 MaxDebrisCount = 500, float DebrisLifetime = 30.0f);

    /**
     * @brief Implement dynamic physics LOD system
     * 
     * Distance-based physics quality scaling:
     * - LOD0 (0-50m): Full physics simulation
     * - LOD1 (50-150m): Reduced substeps, simplified collision
     * - LOD2 (150-300m): Basic physics, no destruction
     * - LOD3 (300m+): Kinematic only, no physics simulation
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsLOD();

    /**
     * @brief Optimize physics substep count based on performance
     * 
     * Adaptive substep scaling:
     * - Monitor frame time
     * - Reduce substeps if frame time > target
     * - Increase substeps if performance allows
     * - Maintain minimum quality threshold
     * 
     * @param TargetFrameTimeMS Target frame time in milliseconds
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void AdaptiveSubstepScaling(float TargetFrameTimeMS = 16.67f);

    /**
     * @brief Get current physics performance metrics
     * 
     * @return Detailed physics performance data
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Performance")
    FPhysicsPerformanceMetrics GetPhysicsPerformanceMetrics() const;

    /**
     * @brief Force cleanup of physics objects beyond distance threshold
     * 
     * @param DistanceThreshold Distance beyond which to cleanup physics objects
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void CleanupDistantPhysicsObjects(float DistanceThreshold = 1000.0f);

    /**
     * @brief Enable/disable physics simulation for specific object types
     * 
     * @param ObjectType Type of physics object to toggle
     * @param bEnabled Whether to enable physics for this object type
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void TogglePhysicsObjectType(EPhysicsObjectType ObjectType, bool bEnabled);

protected:
    /** Physics LOD distance thresholds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics LOD")
    TMap<EPhysicsLODLevel, float> PhysicsLODDistances = {
        {EPhysicsLODLevel::LOD0_Full, 50.0f},
        {EPhysicsLODLevel::LOD1_Reduced, 150.0f},
        {EPhysicsLODLevel::LOD2_Basic, 300.0f},
        {EPhysicsLODLevel::LOD3_Kinematic, 500.0f}
    };

    /** Maximum concurrent ragdolls per platform */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll Performance")
    int32 MaxConcurrentRagdolls = 8;

    /** Maximum destruction debris count */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Performance")
    int32 MaxDestructionDebris = 500;

    /** Destruction debris lifetime in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Performance")
    float DestructionDebrisLifetime = 30.0f;

    /** Physics substep range for adaptive scaling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Physics")
    int32 MinPhysicsSubsteps = 2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Physics")
    int32 MaxPhysicsSubsteps = 8;

    /** Current physics substep count */
    UPROPERTY(BlueprintReadOnly, Category = "Adaptive Physics")
    int32 CurrentPhysicsSubsteps = 6;

    /** Physics performance budget in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Budget")
    float PhysicsPerformanceBudgetMS = 5.0f; // 5ms of 16.67ms frame budget

    /** Enable physics object type toggles */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Object Management")
    TMap<EPhysicsObjectType, bool> PhysicsObjectTypeEnabled = {
        {EPhysicsObjectType::CreatureRagdolls, true},
        {EPhysicsObjectType::DestructionDebris, true},
        {EPhysicsObjectType::EnvironmentalProps, true},
        {EPhysicsObjectType::ProjectilePhysics, true},
        {EPhysicsObjectType::VehiclePhysics, true}
    };

    /** Current performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPhysicsPerformanceMetrics CurrentMetrics;

    /** Physics object tracking arrays */
    UPROPERTY()
    TArray<TWeakObjectPtr<USkeletalMeshComponent>> TrackedRagdolls;

    UPROPERTY()
    TArray<TWeakObjectPtr<UGeometryCollectionComponent>> TrackedDestructionComponents;

    UPROPERTY()
    TArray<TWeakObjectPtr<UPrimitiveComponent>> TrackedPhysicsObjects;

    /** Performance monitoring */
    float LastPerformanceCheckTime = 0.0f;
    float PerformanceCheckInterval = 0.1f; // Check every 100ms

private:
    /** Helper functions */
    void SetDefaultPerformanceSettings();
    void SetPlatformSpecificSettings();
    void InitializePhysicsObjectTracking();
    void SetupPerformanceMonitoring();
    void UpdatePerformanceMetrics();
    void CleanupOldDebris(float MaxLifetime);
    EPhysicsLODLevel GetPhysicsLODForDistance(float Distance) const;
    void ApplyPhysicsLOD(UPrimitiveComponent* Component, EPhysicsLODLevel LODLevel);
};