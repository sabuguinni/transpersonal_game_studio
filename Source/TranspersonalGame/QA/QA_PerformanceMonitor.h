#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
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
    float GameThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 Triangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ComponentCount;

    FQA_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
    }
};

USTRUCT(BlueprintType)
struct FQA_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MinFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxGameThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxRenderThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxGPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxTriangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxActorCount;

    FQA_PerformanceThresholds()
    {
        MinFrameRate = 30.0f;
        MaxFrameTime = 33.33f;
        MaxGameThreadTime = 16.67f;
        MaxRenderThreadTime = 16.67f;
        MaxGPUTime = 16.67f;
        MaxDrawCalls = 2000;
        MaxTriangles = 1000000;
        MaxMemoryUsageMB = 4096.0f;
        MaxActorCount = 10000;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_PerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQA_PerformanceMonitor();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FQA_PerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    TArray<FQA_PerformanceMetrics> GetMetricsHistory();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool CheckPerformanceThresholds();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void SetPerformanceThresholds(const FQA_PerformanceThresholds& NewThresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FQA_PerformanceThresholds GetPerformanceThresholds() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void ClearMetricsHistory();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    TArray<FQA_PerformanceMetrics> MetricsHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    FQA_PerformanceThresholds PerformanceThresholds;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float TimeSinceLastSample;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    int32 MaxHistorySize;

private:
    void UpdateMetrics();
    void LogPerformanceWarning(const FString& Warning);
    float CalculateAverageFrameRate();
    float CalculateAverageFrameTime();
};