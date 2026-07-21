#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Perf_MemoryManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_MemoryPressure : uint8
{
    Low         UMETA(DisplayName = "Low Memory Pressure"),
    Medium      UMETA(DisplayName = "Medium Memory Pressure"),
    High        UMETA(DisplayName = "High Memory Pressure"),
    Critical    UMETA(DisplayName = "Critical Memory Pressure")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float UsedMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float AvailableMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float TotalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float MemoryUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    EPerf_MemoryPressure MemoryPressure = EPerf_MemoryPressure::Low;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    int32 ActiveActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    int32 LoadedAssetCount = 0;
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_MemoryManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_MemoryManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Memory monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    FPerf_MemoryStats GetCurrentMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    bool IsMemoryPressureHigh() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void OptimizeMemoryUsage();

    // Memory threshold management
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void SetMemoryThresholds(float MediumThreshold, float HighThreshold, float CriticalThreshold);

    // Asset management for memory optimization
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void UnloadUnusedAssets();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void PreloadCriticalAssets();

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|Memory", CallInEditor)
    void LogMemoryReport();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void EnableMemoryTracking(bool bEnable);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Thresholds", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MediumPressureThreshold = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Thresholds", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float HighPressureThreshold = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Thresholds", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float CriticalPressureThreshold = 95.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Management")
    float MemoryCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Management")
    bool bAutoGarbageCollection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Management")
    bool bAutoUnloadAssets = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Management")
    int32 MaxActiveActors = 20000;

private:
    // Internal state
    float LastMemoryCheck = 0.0f;
    FPerf_MemoryStats CachedMemoryStats;
    bool bMemoryTrackingEnabled = true;

    // Internal functions
    void UpdateMemoryStats();
    EPerf_MemoryPressure CalculateMemoryPressure(float UsagePercent);
    void HandleMemoryPressure(EPerf_MemoryPressure Pressure);
    void PerformMemoryOptimization();
};