#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Perf_PerformanceProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float DeltaTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float FPS = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly)
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float CPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 VisibleActors = 0;

    FPerf_FrameData()
    {
        DeltaTime = 0.0f;
        FPS = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        GPUTime = 0.0f;
        CPUTime = 0.0f;
        MemoryUsageMB = 0.0f;
        ActiveActors = 0;
        VisibleActors = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinimumFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxGPUTimeMS = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxCPUTimeMS = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTriangles = 2000000;

    FPerf_PerformanceThresholds()
    {
        TargetFPS = 60.0f;
        MinimumFPS = 30.0f;
        MaxGPUTimeMS = 16.67f;
        MaxCPUTimeMS = 16.67f;
        MaxMemoryUsageMB = 4096.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 2000000;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceData();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameData GetCurrentFrameData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameData GetAverageFrameData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinThresholds() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetPerformanceWarnings() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceThresholds(const FPerf_PerformanceThresholds& NewThresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceThresholds GetPerformanceThresholds() const;

    // Automatic Performance Adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerPerformanceOptimization();

    // Debug and Logging
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void DumpPerformanceReport() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsProfilingActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bAutoOptimizationEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FrameData CurrentFrameData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceThresholds PerformanceThresholds;

    UPROPERTY()
    TArray<FPerf_FrameData> FrameHistory;

    UPROPERTY()
    float ProfilingStartTime;

    UPROPERTY()
    int32 FrameCount;

    // Internal Methods
    void CollectFrameData();
    void CollectRenderStats();
    void CollectMemoryStats();
    void CollectActorStats();
    void AnalyzePerformance();
    void ApplyAutoOptimizations();
    float CalculateAverageFPS() const;
    float CalculateAverageGPUTime() const;
    float CalculateAverageCPUTime() const;
    void TrimFrameHistory();

private:
    static const int32 MaxFrameHistorySize = 300; // 5 seconds at 60fps
};