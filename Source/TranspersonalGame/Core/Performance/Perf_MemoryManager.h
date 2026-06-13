#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/PlatformMemory.h"
#include "SharedTypes.h"
#include "Perf_MemoryManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PhysicalMemoryUsedMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float VirtualMemoryUsedMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailablePhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryPressurePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActiveActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 LoadedTextureCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TextureMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bIsMemoryPressureHigh = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float WarningThresholdPercent = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float CriticalThresholdPercent = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MaxTextureMemoryMB = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxActiveActors = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bEnableAutomaticCleanup = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float CleanupInterval = 30.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryPressureChanged, bool, bIsHighPressure);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryThresholdExceeded, EPerf_MemoryPressureLevel, PressureLevel);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_MemoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_MemoryManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    FPerf_MemoryStats GetCurrentMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void UpdateMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    bool IsMemoryPressureHigh() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    EPerf_MemoryPressureLevel GetMemoryPressureLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void SetMemoryThresholds(const FPerf_MemoryThresholds& NewThresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    FPerf_MemoryThresholds GetMemoryThresholds() const { return MemoryThresholds; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void TriggerMemoryCleanup();

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    void SetAutoCleanupEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance|Memory")
    float GetMemoryUsagePercent() const;

    UPROPERTY(BlueprintAssignable, Category = "Performance|Memory")
    FOnMemoryPressureChanged OnMemoryPressureChanged;

    UPROPERTY(BlueprintAssignable, Category = "Performance|Memory")
    FOnMemoryThresholdExceeded OnMemoryThresholdExceeded;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    FPerf_MemoryThresholds MemoryThresholds;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Memory")
    FPerf_MemoryStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    float UpdateInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    bool bIsMonitoringEnabled = true;

private:
    FTimerHandle MemoryUpdateTimer;
    EPerf_MemoryPressureLevel LastPressureLevel;
    bool bWasHighPressure;

    void StartMemoryMonitoring();
    void StopMemoryMonitoring();
    void OnMemoryUpdateTimer();
    void CheckMemoryThresholds();
    void PerformAutomaticCleanup();
    float CalculateMemoryPressure() const;
    void UpdateActorCounts();
    void UpdateTextureCounts();
};