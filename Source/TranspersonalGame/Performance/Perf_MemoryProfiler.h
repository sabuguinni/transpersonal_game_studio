#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Perf_MemoryProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedPhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedVirtualMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PeakUsedPhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TotalPhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailablePhysicalMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ComponentCount;

    FPerf_MemoryStats()
    {
        UsedPhysicalMB = 0.0f;
        UsedVirtualMB = 0.0f;
        PeakUsedPhysicalMB = 0.0f;
        TotalPhysicalMB = 0.0f;
        AvailablePhysicalMB = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_MemoryProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_MemoryProfiler();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    FPerf_MemoryStats GetCurrentMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void StartMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void StopMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    bool IsMemoryProfilingActive() const { return bIsProfilingActive; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    float GetMemoryUsageThresholdMB() const { return MemoryThresholdMB; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void SetMemoryUsageThresholdMB(float ThresholdMB) { MemoryThresholdMB = ThresholdMB; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    bool IsMemoryUsageAboveThreshold() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void LogMemoryReport();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void ForceGarbageCollection();

protected:
    void UpdateMemoryStats();
    void CheckMemoryThreshold();

private:
    UPROPERTY()
    FPerf_MemoryStats CurrentStats;

    UPROPERTY()
    bool bIsProfilingActive;

    UPROPERTY()
    float MemoryThresholdMB;

    UPROPERTY()
    float ProfilingInterval;

    FTimerHandle ProfilingTimerHandle;
};