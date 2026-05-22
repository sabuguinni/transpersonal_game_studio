#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_PerformanceProfiler.generated.h"

UENUM(BlueprintType)
enum class EPerf_ProfilerMode : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Basic          UMETA(DisplayName = "Basic Monitoring"),
    Advanced       UMETA(DisplayName = "Advanced Profiling"),
    Diagnostic     UMETA(DisplayName = "Diagnostic Mode")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsageMB = 0.0f;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        ActorCount = 0;
        DrawCalls = 0;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_ActorPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString ActorName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceCritical = false;

    FPerf_ActorPerformanceData()
    {
        ActorName = TEXT("Unknown");
        RenderTime = 0.0f;
        TickTime = 0.0f;
        TriangleCount = 0;
        bIsPerformanceCritical = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_ProfilerMode ProfilerMode = EPerf_ProfilerMode::Basic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float UpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableDetailedProfiling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bLogPerformanceWarnings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float FPSWarningThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MemoryWarningThresholdMB = 2048.0f;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<FPerf_ActorPerformanceData> ActorPerformanceData;

    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FPerf_ActorPerformanceData> GetActorPerformanceData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetProfilerMode(EPerf_ProfilerMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceTest();

private:
    // Internal profiling state
    bool bIsProfilingActive = false;
    float TimeSinceLastUpdate = 0.0f;
    TArray<float> FPSHistory;
    float TotalFPSSum = 0.0f;
    int32 FPSHistoryCount = 0;

    // Performance monitoring functions
    void UpdatePerformanceMetrics();
    void CollectActorPerformanceData();
    void CheckPerformanceThresholds();
    void LogPerformanceWarning(const FString& Warning);
    float CalculateAverageFPS() const;
    void UpdateFPSHistory(float CurrentFPS);
};