#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/Queue.h"
#include "ConsciousnessPerformanceManager.generated.h"

DECLARE_STATS_GROUP(TEXT("Consciousness Performance"), STATGROUP_Consciousness, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Consciousness State Updates"), STAT_ConsciousnessStateUpdates, STATGROUP_Consciousness);
DECLARE_CYCLE_STAT(TEXT("Reality Shift Processing"), STAT_RealityShiftProcessing, STATGROUP_Consciousness);
DECLARE_CYCLE_STAT(TEXT("Perception Engine Tick"), STAT_PerceptionEngineTick, STATGROUP_Consciousness);
DECLARE_MEMORY_STAT(TEXT("Consciousness Memory Usage"), STAT_ConsciousnessMemory, STATGROUP_Consciousness);

USTRUCT(BlueprintType)
struct TRANSPERSONALENGINE_API FConsciousnessPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float StateUpdateTime;

    UPROPERTY(BlueprintReadOnly)
    float RealityShiftTime;

    UPROPERTY(BlueprintReadOnly)
    float PerceptionTime;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveConsciousnessEntities;

    UPROPERTY(BlueprintReadOnly)
    int32 MemoryUsageKB;

    UPROPERTY(BlueprintReadOnly)
    float AverageFrameTime;

    FConsciousnessPerformanceMetrics()
        : StateUpdateTime(0.0f)
        , RealityShiftTime(0.0f)
        , PerceptionTime(0.0f)
        , ActiveConsciousnessEntities(0)
        , MemoryUsageKB(0)
        , AverageFrameTime(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALENGINE_API FConsciousnessUpdateBatch
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<class UConsciousnessComponent*> Components;

    UPROPERTY()
    float DeltaTime;

    UPROPERTY()
    int32 Priority;

    FConsciousnessUpdateBatch()
        : DeltaTime(0.0f)
        , Priority(0)
    {}
};

/**
 * Performance manager for consciousness systems
 * Handles batching, threading, and optimization of consciousness-related operations
 */
UCLASS(ClassGroup=(TranspersonalEngine), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALENGINE_API UConsciousnessPerformanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessPerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxConsciousnessUpdatesPerFrame = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ConsciousnessUpdateBudgetMS = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableMultithreading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DistanceLODThreshold = 5000.0f;

    // Batching System
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterConsciousnessComponent(class UConsciousnessComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterConsciousnessComponent(class UConsciousnessComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void BatchUpdateConsciousnessComponents(float DeltaTime);

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ClearUnusedConsciousnessData();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FConsciousnessPerformanceMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(float TargetFrameTimeMS);

    // LOD System
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateConsciousnessLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetConsciousnessLODLevel(const FVector& Position) const;

private:
    // Component Management
    UPROPERTY()
    TArray<class UConsciousnessComponent*> RegisteredComponents;

    UPROPERTY()
    TQueue<FConsciousnessUpdateBatch> UpdateQueue;

    // Performance Tracking
    FConsciousnessPerformanceMetrics CurrentMetrics;
    TArray<float> FrameTimeHistory;
    float TargetFrameTimeMS;

    // Threading
    FThreadSafeBool bIsUpdating;
    FCriticalSection ComponentListLock;

    // Memory Management
    int32 LastMemoryCheckFrame;
    const int32 MemoryCheckInterval = 300; // Check every 5 seconds at 60fps

    // LOD System
    TMap<class UConsciousnessComponent*, int32> ComponentLODLevels;

    // Internal Methods
    void ProcessUpdateBatch(const FConsciousnessUpdateBatch& Batch);
    void UpdatePerformanceMetrics(float DeltaTime);
    bool ShouldUpdateComponent(class UConsciousnessComponent* Component, float DeltaTime) const;
    void OptimizeComponentUpdate(class UConsciousnessComponent* Component, int32 LODLevel);
};