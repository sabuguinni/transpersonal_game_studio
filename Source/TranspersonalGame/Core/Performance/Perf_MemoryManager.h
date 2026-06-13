#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Perf_MemoryManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_MemoryLevel : uint8
{
    Low = 0,
    Medium = 1,
    High = 2,
    Critical = 3
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float PhysicalMemoryUsedMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float VirtualMemoryUsedMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float AvailablePhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float TextureMemoryUsedMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    int32 ActiveActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    int32 ActiveComponentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    EPerf_MemoryLevel CurrentMemoryLevel = EPerf_MemoryLevel::Low;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_MemoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_MemoryManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance Memory")
    FPerf_MemoryStats GetCurrentMemoryStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance Memory")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance Memory")
    void SetMemoryOptimizationLevel(EPerf_MemoryLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance Memory")
    bool IsMemoryUsageHigh() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Memory")
    void CleanupUnusedAssets();

    UFUNCTION(BlueprintCallable, Category = "Performance Memory")
    void ReduceTextureQuality(float ReductionFactor = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Performance Memory")
    void LimitActorCount(int32 MaxActors = 8000);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Memory Management")
    EPerf_MemoryLevel CurrentOptimizationLevel = EPerf_MemoryLevel::Medium;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Management")
    float MemoryCheckInterval = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Management")
    float HighMemoryThresholdMB = 6000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Management")
    float CriticalMemoryThresholdMB = 8000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Management")
    bool bAutoOptimizeMemory = true;

private:
    FTimerHandle MemoryCheckTimer;
    
    void CheckMemoryUsage();
    void AutoOptimizeIfNeeded();
    EPerf_MemoryLevel CalculateMemoryLevel(const FPerf_MemoryStats& Stats) const;
};