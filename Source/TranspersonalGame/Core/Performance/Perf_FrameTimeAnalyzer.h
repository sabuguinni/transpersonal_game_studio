#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Perf_FrameTimeAnalyzer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (60+ FPS)"),
    High        UMETA(DisplayName = "High (45-60 FPS)"),
    Medium      UMETA(DisplayName = "Medium (30-45 FPS)"),
    Low         UMETA(DisplayName = "Low (20-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<20 FPS)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameMetrics
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
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel = EPerf_PerformanceLevel::High;

    FPerf_FrameMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        FrameTime = 16.67f;
        GameThreadTime = 8.0f;
        RenderThreadTime = 8.0f;
        GPUTime = 12.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_FrameTimeAnalyzer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_FrameTimeAnalyzer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableFrameAnalysis = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float SampleInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 MaxSamples = 300;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bLogPerformanceWarnings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float CriticalFPSThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float LowFPSThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float HighFPSThreshold = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float UltraFPSThreshold = 60.0f;

    // Current performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_FrameMetrics CurrentMetrics;

    // Performance analysis functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartFrameAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopFrameAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    // Performance optimization triggers
    UFUNCTION(BlueprintImplementableEvent, Category = "Performance Events")
    void OnPerformanceLevelChanged(EPerf_PerformanceLevel NewLevel, EPerf_PerformanceLevel OldLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance Events")
    void OnCriticalPerformance(float CurrentFPS);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance Events")
    void OnPerformanceImproved(float CurrentFPS);

private:
    // Internal tracking
    TArray<float> FPSSamples;
    TArray<float> FrameTimeSamples;
    float SampleTimer;
    bool bAnalysisActive;
    EPerf_PerformanceLevel LastPerformanceLevel;
    
    // Performance calculation helpers
    void UpdateFrameMetrics(float DeltaTime);
    void CalculateAverages();
    EPerf_PerformanceLevel CalculatePerformanceLevel(float FPS) const;
    void CheckPerformanceThresholds();
    float GetEngineFrameTime() const;
    float GetGameThreadTime() const;
    float GetRenderThreadTime() const;
    float GetGPUTime() const;
};

#include "Perf_FrameTimeAnalyzer.generated.h"