#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Perf_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeltaTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    FPerf_FrameData()
    {
        DeltaTime = 0.0f;
        FrameRate = 0.0f;
        ActorCount = 0;
        MemoryUsageMB = 0.0f;
        RenderThreadTime = 0.0f;
        GameThreadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutoLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusion = true;

    FPerf_OptimizationSettings()
    {
        TargetFrameRate = 60.0f;
        MinFrameRate = 30.0f;
        MaxActorsPerFrame = 1000;
        CullingDistance = 10000.0f;
        bEnableAutoLOD = true;
        bEnableOcclusion = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FrameData CurrentFrameData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FPerf_FrameData> FrameHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxFrameHistorySize = 300;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceCritical = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFrameRateRecorded = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFrameRateRecorded = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceData();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameData GetCurrentPerformanceData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceTest();

private:
    bool bMonitoringActive = false;
    float MonitoringStartTime = 0.0f;
    float TotalFrameTime = 0.0f;
    int32 FrameCount = 0;

    void UpdateFrameData(float DeltaTime);
    void CheckPerformanceThresholds();
    void OptimizeLODSettings();
    void OptimizeActorCulling();
    float GetMemoryUsage() const;
    int32 GetActiveActorCount() const;
};