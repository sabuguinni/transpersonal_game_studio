#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "QA_CriticalSystemMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_SystemHealthData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    bool bIsHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    float HealthScore;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    float LastCheckTime;

    FQA_SystemHealthData()
    {
        SystemName = TEXT("Unknown");
        bIsHealthy = false;
        HealthScore = 0.0f;
        LastError = TEXT("");
        LastCheckTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveParticleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FQA_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 0.0f;
        MaxFPS = 0.0f;
        TotalActorCount = 0;
        ActiveParticleCount = 0;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_CriticalSystemMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_CriticalSystemMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core monitoring functions
    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    void StartSystemMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    void StopSystemMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    void RunCriticalSystemCheck();

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    void UpdatePerformanceMetrics();

    // System health checks
    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    bool CheckVFXSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    bool CheckCharacterSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    bool CheckWorldGenerationHealth();

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    bool CheckAudioSystemHealth();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    bool IsPerformanceAcceptable() const;

    // Alert system
    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    void TriggerCriticalAlert(const FString& AlertMessage);

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    void LogSystemWarning(const FString& SystemName, const FString& Warning);

    // Data access
    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    TArray<FQA_SystemHealthData> GetSystemHealthData() const;

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    FQA_PerformanceMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    bool AreAllSystemsHealthy() const;

protected:
    // Monitoring configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MinAcceptableFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxAcceptableActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bEnableSystemHealthChecks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bLogDetailedMetrics;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "QA Runtime")
    TArray<FQA_SystemHealthData> SystemHealthArray;

    UPROPERTY(BlueprintReadOnly, Category = "QA Runtime")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "QA Runtime")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "QA Runtime")
    float LastMonitoringTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA Runtime")
    int32 CriticalAlertCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA Runtime")
    TArray<FString> RecentAlerts;

private:
    // Internal monitoring functions
    void UpdateSystemHealth(const FString& SystemName, bool bHealthy, float Score, const FString& ErrorMsg = TEXT(""));
    void CalculateAverageFPS(float NewFPS);
    void CheckMemoryUsage();
    void ValidateActorIntegrity();
    void MonitorComponentHealth();

    // Performance tracking
    TArray<float> FPSHistory;
    float FPSSum;
    int32 FPSCount;
    float LastFPSUpdate;
};