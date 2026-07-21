#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_PerformanceProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetric
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString MetricName;

    UPROPERTY(BlueprintReadOnly)
    float CurrentValue;

    UPROPERTY(BlueprintReadOnly)
    float AverageValue;

    UPROPERTY(BlueprintReadOnly)
    float MinValue;

    UPROPERTY(BlueprintReadOnly)
    float MaxValue;

    UPROPERTY(BlueprintReadOnly)
    float TargetValue;

    UPROPERTY(BlueprintReadOnly)
    bool bIsWithinTarget;

    FEng_PerformanceMetric()
    {
        CurrentValue = 0.0f;
        AverageValue = 0.0f;
        MinValue = FLT_MAX;
        MaxValue = -FLT_MAX;
        TargetValue = 0.0f;
        bIsWithinTarget = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SystemName;

    UPROPERTY(BlueprintReadOnly)
    float CPUTime;

    UPROPERTY(BlueprintReadOnly)
    float GPUTime;

    UPROPERTY(BlueprintReadOnly)
    int32 MemoryUsage;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly)
    bool bIsActive;

    FEng_SystemProfile()
    {
        CPUTime = 0.0f;
        GPUTime = 0.0f;
        MemoryUsage = 0;
        DrawCalls = 0;
        bIsActive = true;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PerformanceProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_PerformanceProfiler();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterMetric(const FString& MetricName, float TargetValue);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateMetric(const FString& MetricName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetric GetMetric(const FString& MetricName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FEng_PerformanceMetric> GetAllMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ProfileSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_SystemProfile GetSystemProfile(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FEng_SystemProfile> GetAllSystemProfiles();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(const FString& Platform, float TargetFPS, int32 TargetMemoryMB);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinTargets();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerGarbageCollection();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FEng_PerformanceMetric> Metrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FEng_SystemProfile> SystemProfiles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsProfiling;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bAutoOptimization;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TargetMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString CurrentPlatform;

private:
    FDateTime ProfilingStartTime;
    TMap<FString, TArray<float>> MetricHistory;
    
    void UpdateMetricStatistics(FEng_PerformanceMetric& Metric, float NewValue);
    void CheckPerformanceThresholds();
    void OptimizeIfNeeded();
};