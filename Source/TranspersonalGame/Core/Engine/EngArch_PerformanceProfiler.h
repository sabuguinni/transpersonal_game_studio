#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "EngArch_PerformanceProfiler.generated.h"

UENUM(BlueprintType)
enum class EEng_PerformanceCategory : uint8
{
    Rendering,
    Physics,
    AI,
    Audio,
    Networking,
    Memory,
    CPU,
    GPU
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetric
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EEng_PerformanceCategory Category = EEng_PerformanceCategory::CPU;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString MetricName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString Unit;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsWarning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsCritical = false;

    FEng_PerformanceMetric()
    {
        Category = EEng_PerformanceCategory::CPU;
        MetricName = TEXT("");
        CurrentValue = 0.0f;
        AverageValue = 0.0f;
        MaxValue = 0.0f;
        MinValue = 0.0f;
        Unit = TEXT("");
        bIsWarning = false;
        bIsCritical = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage = 0.0f;

    FEng_PerformanceSnapshot()
    {
        Timestamp = 0.0f;
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        CPUTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsage = 0.0f;
    }
};

/**
 * Engine Architect Performance Profiler
 * Monitors and analyzes game performance metrics for architectural decisions
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngArch_PerformanceProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_PerformanceProfiler();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_PerformanceSnapshot GetCurrentSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_PerformanceMetric> GetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetAverageFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    float GetAverageFrameTime() const;

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FString> GetPerformanceWarnings();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void LogPerformanceReport();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetTargetFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetProfilingInterval(float IntervalSeconds);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceWarning, FString, WarningMessage);
    UPROPERTY(BlueprintAssignable, Category = "Engine Architecture")
    FOnPerformanceWarning OnPerformanceWarning;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceCritical, FString, CriticalMessage);
    UPROPERTY(BlueprintAssignable, Category = "Engine Architecture")
    FOnPerformanceCritical OnPerformanceCritical;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsProfilingActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ProfilingInterval = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastProfilingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FEng_PerformanceSnapshot> PerformanceHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<FEng_PerformanceMetric> CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxHistorySize = 300; // 5 minutes at 1 second intervals

private:
    void UpdatePerformanceMetrics();
    void AnalyzePerformance();
    void CheckPerformanceThresholds();
    FEng_PerformanceMetric CreateMetric(EEng_PerformanceCategory Category, const FString& Name, float Value, const FString& Unit);
    void AddPerformanceSnapshot(const FEng_PerformanceSnapshot& Snapshot);
    float CalculateAverageFromHistory(TFunction<float(const FEng_PerformanceSnapshot&)> Extractor) const;
};