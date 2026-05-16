#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/CollisionProfile.h"
#include "SharedTypes.h"
#include "Perf_CollisionOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CollisionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Collision Metrics")
    int32 TotalCollisionActors;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Metrics")
    int32 ComplexCollisionActors;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Metrics")
    int32 SimpleCollisionActors;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Metrics")
    float AverageCollisionChecksPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Collision Metrics")
    float CollisionCPUTime;

    FPerf_CollisionMetrics()
    {
        TotalCollisionActors = 0;
        ComplexCollisionActors = 0;
        SimpleCollisionActors = 0;
        AverageCollisionChecksPerFrame = 0.0f;
        CollisionCPUTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_CollisionLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision LOD")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision LOD")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision LOD")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision LOD")
    float CullingDistance;

    FPerf_CollisionLODSettings()
    {
        HighDetailDistance = 1000.0f;
        MediumDetailDistance = 2500.0f;
        LowDetailDistance = 5000.0f;
        CullingDistance = 10000.0f;
    }
};

/**
 * Performance optimizer for collision detection systems
 * Manages collision complexity based on distance and importance
 * Implements LOD system for collision shapes and detection
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CollisionOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CollisionOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core optimization functions
    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    void OptimizeCollisionForActor(AActor* Actor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    void UpdateCollisionLOD(AActor* Actor, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    void BatchOptimizeCollisions(const TArray<AActor*>& Actors);

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    FPerf_CollisionMetrics AnalyzeCollisionPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance Analysis")
    void ProfileCollisionSystem(float Duration);

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    EPerf_LODLevel CalculateCollisionLOD(float Distance, bool bIsImportant = false);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetCollisionLODSettings(const FPerf_CollisionLODSettings& NewSettings);

    // Culling and optimization
    UFUNCTION(BlueprintCallable, Category = "Culling")
    void CullDistantCollisions(float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void EnableCollisionBatching(bool bEnable);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    float GetCollisionCPUTime() const { return CollisionCPUTime; }

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    int32 GetActiveCollisionCount() const { return ActiveCollisionCount; }

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    bool IsCollisionPerformanceOptimal() const;

protected:
    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_CollisionLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float OptimizationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxCollisionsPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableCollisionBatching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDistanceCulling;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionCPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveCollisionCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CulledCollisionCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    // Internal state
    float LastOptimizationTime;
    TArray<TWeakObjectPtr<AActor>> TrackedActors;
    TMap<AActor*, EPerf_LODLevel> ActorLODMap;
    
    // Performance targets
    static constexpr float TARGET_COLLISION_CPU_TIME = 2.0f; // 2ms for 60fps
    static constexpr int32 MAX_COLLISION_ACTORS = 2000;

private:
    void UpdateCollisionComplexity(UPrimitiveComponent* Component, EPerf_LODLevel LODLevel);
    void DisableCollisionForActor(AActor* Actor);
    void EnableCollisionForActor(AActor* Actor);
    float CalculateCollisionImportance(AActor* Actor);
    void OptimizeCollisionChannels(UPrimitiveComponent* Component);
};