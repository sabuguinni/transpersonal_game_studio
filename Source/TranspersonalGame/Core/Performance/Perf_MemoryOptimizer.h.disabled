#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Engine/Engine.h"
#include "Perf_MemoryOptimizer.generated.h"

DECLARE_STATS_GROUP(TEXT("Perf Memory"), STATGROUP_PerfMemory, STATCAT_Advanced);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Objects"), STAT_PerfActiveObjects, STATGROUP_PerfMemory);
DECLARE_MEMORY_STAT(TEXT("Physics Memory"), STAT_PerfPhysicsMemory, STATGROUP_PerfMemory);
DECLARE_MEMORY_STAT(TEXT("Mesh Memory"), STAT_PerfMeshMemory, STATGROUP_PerfMemory);

UENUM(BlueprintType)
enum class EPerf_MemoryPriority : uint8
{
    Critical = 0,
    High = 1,
    Medium = 2,
    Low = 3,
    Disposable = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float VirtualMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailableMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryPressurePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActiveObjectCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PhysicsMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MeshMemoryMB;

    FPerf_MemoryMetrics()
    {
        PhysicalMemoryMB = 0.0f;
        VirtualMemoryMB = 0.0f;
        UsedMemoryMB = 0.0f;
        AvailableMemoryMB = 0.0f;
        MemoryPressurePercent = 0.0f;
        ActiveObjectCount = 0;
        PhysicsMemoryMB = 0.0f;
        MeshMemoryMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryPool
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Pool")
    FString PoolName;

    UPROPERTY(BlueprintReadOnly, Category = "Pool")
    float AllocatedMB;

    UPROPERTY(BlueprintReadOnly, Category = "Pool")
    float MaxSizeMB;

    UPROPERTY(BlueprintReadOnly, Category = "Pool")
    int32 ActiveAllocations;

    UPROPERTY(BlueprintReadOnly, Category = "Pool")
    EPerf_MemoryPriority Priority;

    FPerf_MemoryPool()
    {
        PoolName = TEXT("Unknown");
        AllocatedMB = 0.0f;
        MaxSizeMB = 0.0f;
        ActiveAllocations = 0;
        Priority = EPerf_MemoryPriority::Medium;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_MemoryOptimizer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_MemoryOptimizer();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Memory monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    FPerf_MemoryMetrics GetCurrentMemoryMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    float GetMemoryPressure();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    bool IsMemoryPressureHigh();

    // Memory optimization
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void PurgeUnusedAssets();

    // Pool management
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void RegisterMemoryPool(const FString& PoolName, float MaxSizeMB, EPerf_MemoryPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    TArray<FPerf_MemoryPool> GetMemoryPools();

    // Memory pressure callbacks
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void SetMemoryPressureThreshold(float ThresholdPercent);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance|Memory")
    void OnMemoryPressureHigh(float CurrentPressure);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance|Memory")
    void OnMemoryPressureNormal(float CurrentPressure);

    // Object tracking
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void TrackObject(UObject* Object, EPerf_MemoryPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void UntrackObject(UObject* Object);

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    int32 GetTrackedObjectCount();

protected:
    // Memory tracking
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FPerf_MemoryMetrics CachedMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryPressureThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bIsMemoryPressureHigh;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float LastOptimizationTime;

    // Memory pools
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FPerf_MemoryPool> MemoryPools;

    // Tracked objects
    UPROPERTY()
    TMap<TWeakObjectPtr<UObject>, EPerf_MemoryPriority> TrackedObjects;

    // Timer handles
    FTimerHandle MemoryMonitorTimer;
    FTimerHandle OptimizationTimer;

private:
    void UpdateMemoryMetrics();
    void CheckMemoryPressure();
    void OptimizeTrackedObjects();
    void CleanupInvalidObjects();
    
    float GetPhysicsMemoryUsage();
    float GetMeshMemoryUsage();
    
    void OptimizeByPriority(EPerf_MemoryPriority Priority);
    void PurgeDisposableObjects();
    
    bool bInitialized;
    float LastMemoryCheckTime;
    float MemoryCheckInterval;
    float OptimizationInterval;
};