#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/Platform.h"
#include "Perf_MemoryProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemorySnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float VirtualMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TextureMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MeshMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AudioMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FDateTime Timestamp;

    FPerf_MemorySnapshot()
    {
        PhysicalMemoryMB = 0.0f;
        VirtualMemoryMB = 0.0f;
        TextureMemoryMB = 0.0f;
        MeshMemoryMB = 0.0f;
        AudioMemoryMB = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
        Timestamp = FDateTime::Now();
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_MemoryProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_MemorySnapshot CaptureMemorySnapshot();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMemoryProfiling(float IntervalSeconds = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_MemorySnapshot> GetMemoryHistory() const { return MemoryHistory; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPeakMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMemoryUsageCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemoryUsage();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FPerf_MemorySnapshot> MemoryHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bIsProfilingActive;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float ProfilingInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryWarningThresholdMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryCriticalThresholdMB;

private:
    FTimerHandle ProfilingTimerHandle;

    void OnProfilingTick();
    float CalculateTextureMemoryUsage() const;
    float CalculateMeshMemoryUsage() const;
    float CalculateAudioMemoryUsage() const;
    void CleanupOldSnapshots();
};