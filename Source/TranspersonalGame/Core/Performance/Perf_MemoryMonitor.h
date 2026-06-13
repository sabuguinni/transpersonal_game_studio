#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Perf_MemoryMonitor.generated.h"

UENUM(BlueprintType)
enum class EPerf_MemoryCategory : uint8
{
    Total       UMETA(DisplayName = "Total Memory"),
    Physics     UMETA(DisplayName = "Physics Memory"),
    Rendering   UMETA(DisplayName = "Rendering Memory"),
    Audio       UMETA(DisplayName = "Audio Memory"),
    Animation   UMETA(DisplayName = "Animation Memory"),
    Textures    UMETA(DisplayName = "Texture Memory"),
    Meshes      UMETA(DisplayName = "Mesh Memory"),
    Scripts     UMETA(DisplayName = "Script Memory")
};

USTRUCT(BlueprintType)
struct FPerf_MemoryUsage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailableMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TotalMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PercentageUsed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float PeakUsageMB = 0.0f;

    FPerf_MemoryUsage()
    {
        UsedMB = 0.0f;
        AvailableMB = 0.0f;
        TotalMB = 0.0f;
        PercentageUsed = 0.0f;
        PeakUsageMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_MemoryAlert
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    EPerf_MemoryCategory Category = EPerf_MemoryCategory::Total;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    float ThresholdMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    float CurrentUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    FString AlertMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    float Timestamp = 0.0f;

    FPerf_MemoryAlert()
    {
        Category = EPerf_MemoryCategory::Total;
        ThresholdMB = 0.0f;
        CurrentUsageMB = 0.0f;
        AlertMessage = TEXT("");
        Timestamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_MemorySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float WarningThresholdPercent = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CriticalThresholdPercent = 95.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MonitoringInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoGarbageCollection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float GCThresholdPercent = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxAlertHistory = 50;

    FPerf_MemorySettings()
    {
        WarningThresholdPercent = 80.0f;
        CriticalThresholdPercent = 95.0f;
        MonitoringInterval = 1.0f;
        bAutoGarbageCollection = true;
        GCThresholdPercent = 85.0f;
        MaxAlertHistory = 50;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_MemoryMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_MemoryMonitor();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

    // Memory monitoring
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_MemoryUsage GetMemoryUsage(EPerf_MemoryCategory Category = EPerf_MemoryCategory::Total) const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    TArray<FPerf_MemoryUsage> GetAllMemoryUsage() const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsMemoryUsageCritical() const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetMemoryPressure() const;

    // Alerts
    UFUNCTION(BlueprintPure, Category = "Performance")
    TArray<FPerf_MemoryAlert> GetActiveAlerts() const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    TArray<FPerf_MemoryAlert> GetAlertHistory() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ClearAlerts();

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMemorySettings(const FPerf_MemorySettings& Settings);

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_MemorySettings GetMemorySettings() const;

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool TryFreeMemory(float TargetMB);

    // Statistics
    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetMemoryUsageTrend() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetStatistics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogMemoryReport() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_MemorySettings MemorySettings;

    UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
    TArray<FPerf_MemoryAlert> ActiveAlerts;

    UPROPERTY(BlueprintReadOnly, Category = "Monitoring")
    TArray<FPerf_MemoryAlert> AlertHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDetailedMonitoring = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bLogMemoryWarnings = true;

private:
    void UpdateMemoryMetrics();
    void CheckMemoryThresholds();
    void HandleMemoryAlert(EPerf_MemoryCategory Category, float UsageMB, float ThresholdMB);
    void PerformAutomaticGC();
    float CalculateMemoryPressure() const;
    FPerf_MemoryUsage CalculateCategoryUsage(EPerf_MemoryCategory Category) const;

    TMap<EPerf_MemoryCategory, FPerf_MemoryUsage> CachedMemoryUsage;
    TArray<float> MemoryUsageHistory;
    float LastMonitoringTime;
    float LastGCTime;
    bool bInitialized;
    int32 MaxHistorySize;
};