#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Perf_PerformanceProfiler.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Critical    UMETA(DisplayName = "Critical (<20 FPS)"),
    Low         UMETA(DisplayName = "Low (20-30 FPS)"),
    Medium      UMETA(DisplayName = "Medium (30-45 FPS)"),
    High        UMETA(DisplayName = "High (45-60 FPS)"),
    Ultra       UMETA(DisplayName = "Ultra (60+ FPS)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel = EPerf_PerformanceLevel::Medium;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFrameTime = 16.67f;
        GameThreadTime = 8.0f;
        RenderThreadTime = 12.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
        PerformanceLevel = EPerf_PerformanceLevel::Medium;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceLevelChanged, EPerf_PerformanceLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFrameTimeSpike, float, SpikeTime);

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float SamplingInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 SampleHistorySize = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableAutomaticProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bLogPerformanceWarnings = true;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float CriticalFPSThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float LowFPSThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MediumFPSThreshold = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float HighFPSThreshold = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float FrameTimeSpikeThreshold = 33.33f; // 30 FPS equivalent

    // Current Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<float> FrameTimeHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<float> FPSHistory;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnPerformanceLevelChanged OnPerformanceLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnFrameTimeSpike OnFrameTimeSpike;

    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SavePerformanceReport(const FString& FilePath);

private:
    float TimeSinceLastSample = 0.0f;
    bool bIsProfiling = false;
    EPerf_PerformanceLevel LastPerformanceLevel = EPerf_PerformanceLevel::Medium;

    void UpdatePerformanceMetrics();
    void UpdatePerformanceLevel();
    void AddFrameTimeSample(float FrameTime);
    void CheckForFrameTimeSpikes(float FrameTime);
    EPerf_PerformanceLevel CalculatePerformanceLevel(float FPS) const;
    void TriggerPerformanceLevelChange(EPerf_PerformanceLevel NewLevel);
};