#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Perf_MemoryManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_MemoryPressure : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float UsedPhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float AvailablePhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float TotalPhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float UsedVirtualMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    float MemoryUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    EPerf_MemoryPressure MemoryPressure = EPerf_MemoryPressure::Low;

    FPerf_MemoryStats()
    {
        UsedPhysicalMemoryMB = 0.0f;
        AvailablePhysicalMemoryMB = 0.0f;
        TotalPhysicalMemoryMB = 0.0f;
        UsedVirtualMemoryMB = 0.0f;
        MemoryUsagePercent = 0.0f;
        MemoryPressure = EPerf_MemoryPressure::Low;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnMemoryPressureChanged, EPerf_MemoryPressure, NewPressure);

UCLASS()
class TRANSPERSONALGAME_API UPerf_MemoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_MemoryManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "Memory Events")
    FPerf_OnMemoryPressureChanged OnMemoryPressureChanged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryCheckInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float HighPressureThreshold = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float CriticalPressureThreshold = 90.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    FPerf_MemoryStats CurrentMemoryStats;

    UPROPERTY(BlueprintReadOnly, Category = "Memory Stats")
    bool bIsMonitoring = false;

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void StartMemoryMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void StopMemoryMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    FPerf_MemoryStats GetMemoryStats();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ClearUnusedAssets();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ReduceTextureQuality();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ReduceAudioQuality();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    float GetMemoryUsagePercent() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    EPerf_MemoryPressure GetCurrentMemoryPressure() const;

private:
    void UpdateMemoryStats();
    void CheckMemoryPressure();
    void HandleMemoryPressure(EPerf_MemoryPressure NewPressure);
    void EmergencyMemoryCleanup();

    FTimerHandle MemoryCheckTimer;
    EPerf_MemoryPressure LastMemoryPressure = EPerf_MemoryPressure::Low;
    
    // Memory optimization tracking
    bool bHasReducedTextureQuality = false;
    bool bHasReducedAudioQuality = false;
    int32 GCCallCount = 0;
    float LastGCTime = 0.0f;
};