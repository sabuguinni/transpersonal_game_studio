#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Perf_MemoryProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedVirtualMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PeakPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 NumLoadedObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 NumStaticMeshes;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 NumTextures;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TextureMemoryMB;

    FPerf_MemoryStats()
    {
        UsedPhysicalMemoryMB = 0.0f;
        UsedVirtualMemoryMB = 0.0f;
        PeakPhysicalMemoryMB = 0.0f;
        NumLoadedObjects = 0;
        NumStaticMeshes = 0;
        NumTextures = 0;
        TextureMemoryMB = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_MemoryProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_MemoryProfiler();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_MemoryStats GetCurrentMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogMemoryReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMemoryUsageHigh();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMemoryWarningThreshold(float ThresholdMB);

protected:
    UPROPERTY()
    bool bIsProfilingActive;

    UPROPERTY()
    float MemoryWarningThresholdMB;

    UPROPERTY()
    TArray<FPerf_MemoryStats> MemoryHistory;

    FTimerHandle ProfilingTimerHandle;

    void UpdateMemoryStats();
    void CheckMemoryThresholds();
    float GetTextureMemoryUsage();
    int32 CountObjectsByClass(UClass* ObjectClass);
};