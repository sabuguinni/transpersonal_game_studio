#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Perf_MemoryManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_MemoryCategory : uint8
{
    Textures UMETA(DisplayName = "Textures"),
    Meshes UMETA(DisplayName = "Meshes"),
    Audio UMETA(DisplayName = "Audio"),
    Animation UMETA(DisplayName = "Animation"),
    Physics UMETA(DisplayName = "Physics"),
    Particles UMETA(DisplayName = "Particles"),
    Scripts UMETA(DisplayName = "Scripts"),
    Other UMETA(DisplayName = "Other")
};

USTRUCT(BlueprintType)
struct FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TotalPhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedPhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailablePhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float GameMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TextureMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MeshMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AudioMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryPressureLevel = 0.0f; // 0.0 = no pressure, 1.0 = critical

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bIsMemoryPressureHigh = false;

    FPerf_MemoryStats()
    {
        TotalPhysicalMemoryMB = 0.0f;
        UsedPhysicalMemoryMB = 0.0f;
        AvailablePhysicalMemoryMB = 0.0f;
        GameMemoryUsageMB = 0.0f;
        TextureMemoryMB = 0.0f;
        MeshMemoryMB = 0.0f;
        AudioMemoryMB = 0.0f;
        MemoryPressureLevel = 0.0f;
        bIsMemoryPressureHigh = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnMemoryPressureChanged, bool, bHighPressure);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_MemoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_MemoryManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FPerf_MemoryStats GetCurrentMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void StartMemoryMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void StopMemoryMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool IsMemoryPressureHigh() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void SetMemoryPressureThreshold(float Threshold);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    float GetMemoryUsageByCategory(EPerf_MemoryCategory Category);

    UPROPERTY(BlueprintAssignable, Category = "Memory")
    FPerf_OnMemoryPressureChanged OnMemoryPressureChanged;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FPerf_MemoryStats CurrentStats;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryPressureThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bAutoOptimizeEnabled;

private:
    void UpdateMemoryStats();
    void CheckMemoryPressure();
    void HandleMemoryPressure();
    void ClearUnusedAssets();
    
    FTimerHandle MemoryUpdateTimer;
    bool bWasHighPressure;
    float LastGCTime;
    
    // Memory tracking
    TMap<EPerf_MemoryCategory, float> CategoryMemoryUsage;
};

#include "Perf_MemoryManager.generated.h"