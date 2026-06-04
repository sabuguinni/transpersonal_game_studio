#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "QA_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ComponentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUTime;

    FQA_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        GPUTime = 0.0f;
        CPUTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_PerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    AQA_PerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool IsPerformanceAcceptable();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoStartMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxAcceptableActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxAcceptableMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    float WarningFrameRateThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    float CriticalFrameRateThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TArray<FQA_PerformanceMetrics> MetricsHistory;

    UPROPERTY(BlueprintReadOnly, Category = "QA Status")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "QA Status")
    bool bPerformanceWarning;

    UPROPERTY(BlueprintReadOnly, Category = "QA Status")
    bool bPerformanceCritical;

private:
    void UpdateMetrics();
    void CheckPerformanceThresholds();
    void LogWarning(const FString& Message);
    void LogCritical(const FString& Message);

    FTimerHandle MonitoringTimer;
    float LastUpdateTime;
    int32 FrameCounter;
    float AccumulatedFrameTime;
};