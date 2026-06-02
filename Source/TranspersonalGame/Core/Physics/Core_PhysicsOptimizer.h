#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Core_PhysicsOptimizer.generated.h"

class UPrimitiveComponent;
class AActor;

/**
 * Physics optimization levels for different performance targets
 */
UENUM(BlueprintType)
enum class ECore_PhysicsOptimizationLevel : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Conservative    UMETA(DisplayName = "Conservative"),
    Balanced        UMETA(DisplayName = "Balanced"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Maximum         UMETA(DisplayName = "Maximum")
};

/**
 * Physics LOD (Level of Detail) settings for distance-based optimization
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsLODSettings
{
    GENERATED_BODY()

    /** Distance threshold for LOD 0 (highest detail) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LOD0Distance = 500.0f;

    /** Distance threshold for LOD 1 (medium detail) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LOD1Distance = 1500.0f;

    /** Distance threshold for LOD 2 (low detail) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LOD2Distance = 3000.0f;

    /** Distance threshold for LOD 3 (minimal detail) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float LOD3Distance = 5000.0f;

    /** Tick rate multiplier for each LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    TArray<float> LODTickRateMultipliers = {1.0f, 0.5f, 0.25f, 0.1f, 0.05f};

    FCore_PhysicsLODSettings()
    {
        LODTickRateMultipliers.SetNum(5);
        LODTickRateMultipliers[0] = 1.0f;   // LOD 0: Full rate
        LODTickRateMultipliers[1] = 0.5f;   // LOD 1: Half rate
        LODTickRateMultipliers[2] = 0.25f;  // LOD 2: Quarter rate
        LODTickRateMultipliers[3] = 0.1f;   // LOD 3: 10% rate
        LODTickRateMultipliers[4] = 0.05f;  // LOD 4: 5% rate
    }
};

/**
 * Physics optimization statistics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsOptimizationStats
{
    GENERATED_BODY()

    /** Total number of physics objects being tracked */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 TotalPhysicsObjects = 0;

    /** Number of objects at each LOD level */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    TArray<int32> ObjectsPerLOD;

    /** Number of objects that were culled this frame */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CulledObjects = 0;

    /** Number of objects that were sleeping */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 SleepingObjects = 0;

    /** Average physics tick time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageTickTime = 0.0f;

    /** Current optimization level */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    ECore_PhysicsOptimizationLevel CurrentOptimizationLevel = ECore_PhysicsOptimizationLevel::Balanced;

    /** Performance gain percentage from optimization */
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float PerformanceGainPercent = 0.0f;

    FCore_PhysicsOptimizationStats()
    {
        ObjectsPerLOD.SetNum(5);
        for (int32 i = 0; i < 5; i++)
        {
            ObjectsPerLOD[i] = 0;
        }
    }
};

/**
 * Core Physics Optimizer Component
 * 
 * Manages physics performance optimization for the prehistoric survival game.
 * Implements distance-based LOD, sleep management, culling, and adaptive quality scaling
 * to maintain stable 60 FPS with hundreds of dinosaurs and environmental physics objects.
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === OPTIMIZATION SETTINGS ===

    /** Enable/disable physics optimization system */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableOptimization = true;

    /** Current optimization level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    ECore_PhysicsOptimizationLevel OptimizationLevel = ECore_PhysicsOptimizationLevel::Balanced;

    /** Physics LOD settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    FCore_PhysicsLODSettings LODSettings;

    /** Target frame rate for adaptive optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (ClampMin = "30", ClampMax = "120"))
    float TargetFrameRate = 60.0f;

    /** Maximum number of physics objects to simulate simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (ClampMin = "100", ClampMax = "10000"))
    int32 MaxSimulatedObjects = 2000;

    /** Enable automatic sleep management for idle objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAutoSleep = true;

    /** Velocity threshold below which objects are put to sleep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float SleepVelocityThreshold = 1.0f;

    /** Time threshold for putting objects to sleep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (ClampMin = "0.5", ClampMax = "10.0"))
    float SleepTimeThreshold = 2.0f;

    // === CULLING SETTINGS ===

    /** Enable distance-based culling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableDistanceCulling = true;

    /** Maximum distance for physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling", meta = (ClampMin = "1000", ClampMax = "20000"))
    float MaxPhysicsDistance = 8000.0f;

    /** Enable frustum culling for physics objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling = true;

    /** Frustum culling margin (extends frustum for physics objects) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FrustumCullingMargin = 0.5f;

    // === ADAPTIVE QUALITY ===

    /** Enable adaptive quality scaling based on performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Quality")
    bool bEnableAdaptiveQuality = true;

    /** Frame time threshold for reducing quality */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Quality", meta = (ClampMin = "10.0", ClampMax = "50.0"))
    float QualityReductionThreshold = 20.0f; // milliseconds

    /** Frame time threshold for increasing quality */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Quality", meta = (ClampMin = "5.0", ClampMax = "25.0"))
    float QualityIncreaseThreshold = 12.0f; // milliseconds

    /** How often to check for quality adjustments (in seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Quality", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float QualityCheckInterval = 1.0f;

    // === PUBLIC INTERFACE ===

    /** Get current optimization statistics */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    FCore_PhysicsOptimizationStats GetOptimizationStats() const;

    /** Set optimization level manually */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void SetOptimizationLevel(ECore_PhysicsOptimizationLevel NewLevel);

    /** Force optimization update for all tracked objects */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void ForceOptimizationUpdate();

    /** Add object to optimization tracking */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void AddTrackedObject(AActor* Actor);

    /** Remove object from optimization tracking */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void RemoveTrackedObject(AActor* Actor);

    /** Get current physics LOD level for a specific object */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    int32 GetObjectLODLevel(AActor* Actor) const;

    /** Check if object is currently being simulated */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    bool IsObjectSimulated(AActor* Actor) const;

protected:
    // === INTERNAL OPTIMIZATION LOGIC ===

    /** Update physics optimization for all tracked objects */
    void UpdatePhysicsOptimization(float DeltaTime);

    /** Calculate LOD level for an object based on distance and importance */
    int32 CalculateObjectLOD(AActor* Actor, float DistanceToPlayer) const;

    /** Apply LOD settings to a physics object */
    void ApplyLODToObject(AActor* Actor, int32 LODLevel);

    /** Update sleep state for physics objects */
    void UpdateSleepManagement(float DeltaTime);

    /** Perform distance and frustum culling */
    void UpdateCulling(float DeltaTime);

    /** Update adaptive quality based on performance */
    void UpdateAdaptiveQuality(float DeltaTime);

    /** Get player location for distance calculations */
    FVector GetPlayerLocation() const;

    /** Check if object is within camera frustum */
    bool IsObjectInFrustum(AActor* Actor) const;

    /** Calculate object importance for optimization priority */
    float CalculateObjectImportance(AActor* Actor) const;

private:
    // === INTERNAL DATA ===

    /** List of actors being tracked for optimization */
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> TrackedObjects;

    /** Current optimization statistics */
    FCore_PhysicsOptimizationStats CurrentStats;

    /** Timer for quality adjustment checks */
    FTimerHandle QualityCheckTimer;

    /** Frame time history for adaptive quality */
    TArray<float> FrameTimeHistory;

    /** Current frame time average */
    float AverageFrameTime = 16.67f; // 60 FPS baseline

    /** Performance monitoring */
    float LastOptimizationTime = 0.0f;
    int32 OptimizationFrameCounter = 0;

    /** Sleep management data */
    TMap<TWeakObjectPtr<AActor>, float> ObjectSleepTimers;
    TMap<TWeakObjectPtr<AActor>, FVector> LastObjectVelocities;

    /** LOD management data */
    TMap<TWeakObjectPtr<AActor>, int32> ObjectLODLevels;
    TMap<TWeakObjectPtr<AActor>, float> ObjectLastUpdateTimes;
};