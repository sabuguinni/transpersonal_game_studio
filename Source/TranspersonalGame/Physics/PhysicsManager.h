// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsImpact, AActor*, ActorA, AActor*, ActorB);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsDestruction, AActor*, DestroyedActor);

/**
 * @brief Core physics manager for the Transpersonal Game
 * 
 * Manages all physics simulation, collision detection, and destruction systems.
 * Optimized for 60fps on console, 120fps on PC with dynamic quality scaling.
 * 
 * Key Features:
 * - Chaos Physics integration with custom collision channels
 * - Dynamic LOD system for physics simulation
 * - Performance budgeting with automatic quality adjustment
 * - Destruction system with debris management
 * - Vehicle physics with realistic handling
 * 
 * Performance Budget: 2.5ms per frame maximum
 * Memory Budget: 128MB for physics simulation data
 * 
 * @author Core Systems Programmer
 * @version 1.0
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Initialize physics world with custom settings
     * @param WorldGravity Gravity vector for the world
     * @param SimulationQuality Quality level (0-3, 3 being highest)
     */
    UFUNCTION(BlueprintCallable, Category = "Physics|Core")
    void InitializePhysicsWorld(FVector WorldGravity = FVector(0, 0, -980.0f), int32 SimulationQuality = 2);

    /**
     * @brief Apply impulse to actor with physics validation
     * @param TargetActor Actor to apply impulse to
     * @param ImpulseVector Impulse direction and magnitude
     * @param bVelChange Whether to treat as velocity change
     * @return True if impulse was successfully applied
     */
    UFUNCTION(BlueprintCallable, Category = "Physics|Forces")
    bool ApplyPhysicsImpulse(AActor* TargetActor, FVector ImpulseVector, bool bVelChange = false);

    /**
     * @brief Create physics-based explosion at location
     * @param ExplosionLocation World location of explosion
     * @param ExplosionRadius Radius of effect
     * @param ExplosionForce Force magnitude
     * @param bCauseDamage Whether explosion should damage actors
     */
    UFUNCTION(BlueprintCallable, Category = "Physics|Destruction")
    void CreatePhysicsExplosion(FVector ExplosionLocation, float ExplosionRadius, float ExplosionForce, bool bCauseDamage = true);

    /**
     * @brief Enable/disable physics simulation for actor
     * @param TargetActor Actor to modify
     * @param bEnablePhysics Whether to enable physics
     */
    UFUNCTION(BlueprintCallable, Category = "Physics|Control")
    void SetActorPhysicsEnabled(AActor* TargetActor, bool bEnablePhysics);

    /**
     * @brief Get current physics performance metrics
     * @param OutFrameTime Current physics frame time in ms
     * @param OutActiveRigidBodies Number of active rigid bodies
     * @param OutMemoryUsage Memory usage in MB
     */
    UFUNCTION(BlueprintCallable, Category = "Physics|Debug")
    void GetPhysicsPerformanceMetrics(float& OutFrameTime, int32& OutActiveRigidBodies, float& OutMemoryUsage);

    /**
     * @brief Adjust physics quality based on performance
     * @param TargetFrameTime Desired frame time in ms
     */
    UFUNCTION(BlueprintCallable, Category = "Physics|Optimization")
    void AdjustPhysicsQuality(float TargetFrameTime = 2.5f);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Physics|Events")
    FOnPhysicsImpact OnPhysicsImpact;

    UPROPERTY(BlueprintAssignable, Category = "Physics|Events")
    FOnPhysicsDestruction OnPhysicsDestruction;

protected:
    // Core physics settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float MaxPhysicsFrameTime = 2.5f; // Maximum allowed physics frame time in ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    int32 MaxActiveRigidBodies = 2048; // Maximum number of active rigid bodies

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float PhysicsLODDistance = 5000.0f; // Distance for physics LOD switching

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnableAsyncPhysics = true; // Enable asynchronous physics simulation

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Debug")
    float CurrentPhysicsFrameTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Debug")
    int32 CurrentActiveRigidBodies = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Debug")
    float CurrentMemoryUsage = 0.0f;

    // Internal systems
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;

    UPROPERTY()
    class UPhysicsLODSystem* PhysicsLODSystem;

private:
    // Performance monitoring
    void UpdatePerformanceMetrics();
    void OptimizePhysicsSimulation();
    
    // Collision handling
    UFUNCTION()
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

    // Memory management
    void CleanupInactivePhysicsObjects();
    
    // Quality scaling
    void ScalePhysicsQuality(float QualityMultiplier);

    // Internal state
    float PerformanceTimer = 0.0f;
    int32 CurrentQualityLevel = 2;
    bool bPerformanceOptimizationEnabled = true;
};

/**
 * @brief Physics LOD system for performance optimization
 * 
 * Manages level-of-detail for physics simulation based on distance from camera
 * and performance requirements.
 */
UCLASS()
class TRANSPERSONALGAME_API UPhysicsLODSystem : public UObject
{
    GENERATED_BODY()

public:
    /**
     * @brief Update LOD levels for all tracked physics objects
     * @param ViewLocation Camera location for distance calculations
     * @param DeltaTime Time since last update
     */
    void UpdatePhysicsLOD(FVector ViewLocation, float DeltaTime);

    /**
     * @brief Register actor for LOD management
     * @param Actor Actor to track
     */
    void RegisterActor(AActor* Actor);

    /**
     * @brief Unregister actor from LOD management
     * @param Actor Actor to stop tracking
     */
    void UnregisterActor(AActor* Actor);

protected:
    struct FPhysicsLODData
    {
        TWeakObjectPtr<AActor> Actor;
        float DistanceToCamera = 0.0f;
        int32 CurrentLODLevel = 0;
        float LastUpdateTime = 0.0f;
    };

    TArray<FPhysicsLODData> TrackedActors;
    
    // LOD distance thresholds
    static constexpr float LOD_DISTANCE_HIGH = 1000.0f;
    static constexpr float LOD_DISTANCE_MEDIUM = 3000.0f;
    static constexpr float LOD_DISTANCE_LOW = 8000.0f;
    static constexpr float LOD_DISTANCE_DISABLED = 15000.0f;

private:
    void ApplyLODLevel(AActor* Actor, int32 LODLevel);
    int32 CalculateLODLevel(float Distance);
};