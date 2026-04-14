#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low = 0,
    Medium = 1,
    High = 2,
    Ultra = 3
};

UENUM(BlueprintType)
enum class EPerf_PerformanceMetric : uint8
{
    FPS = 0,
    FrameTime = 1,
    GPUTime = 2,
    MemoryUsage = 3,
    DrawCalls = 4,
    Triangles = 5
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float MaxFrameTime = 16.67f; // 60 FPS = 16.67ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float MaxGPUTime = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    int32 MaxTriangles = 1000000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Budget")
    float MaxMemoryUsageMB = 4096.0f;

    FPerf_PerformanceBudget()
    {
        TargetFPS = 60.0f;
        MaxFrameTime = 16.67f;
        MaxGPUTime = 12.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 1000000;
        MaxMemoryUsageMB = 4096.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float CurrentFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float CurrentGPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 CurrentDrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 CurrentTriangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float CurrentMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    bool bIsWithinBudget = true;

    FPerf_PerformanceStats()
    {
        CurrentFPS = 0.0f;
        CurrentFrameTime = 0.0f;
        CurrentGPUTime = 0.0f;
        CurrentDrawCalls = 0;
        CurrentTriangles = 0;
        CurrentMemoryUsageMB = 0.0f;
        bIsWithinBudget = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnPerformanceWarning, const FString&, WarningMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPerf_OnPerformanceLevelChanged, EPerf_PerformanceLevel, OldLevel, EPerf_PerformanceLevel, NewLevel);

/**
 * Performance Manager Subsystem
 * Monitors and manages game performance, automatically adjusting quality settings
 * to maintain target frame rates and performance budgets
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceStats();

    // Performance budget management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(const FPerf_PerformanceBudget& NewBudget);

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_PerformanceBudget GetPerformanceBudget() const { return PerformanceBudget; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_PerformanceStats GetCurrentPerformanceStats() const { return CurrentStats; }

    // Performance level management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const { return CurrentPerformanceLevel; }

    // Auto-optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void RunPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void CreatePerformanceStressTest();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FPerf_OnPerformanceWarning OnPerformanceWarning;

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FPerf_OnPerformanceLevelChanged OnPerformanceLevelChanged;

protected:
    // Performance monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Config")
    FPerf_PerformanceBudget PerformanceBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    FPerf_PerformanceStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Config")
    EPerf_PerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Config")
    bool bAutoOptimizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Config")
    float MonitoringUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Config")
    int32 PerformanceHistorySize;

    // Internal state
    FTimerHandle MonitoringTimerHandle;
    TArray<float> FPSHistory;
    TArray<float> FrameTimeHistory;
    float LastOptimizationTime;
    bool bIsMonitoring;

private:
    void ApplyPerformanceLevel(EPerf_PerformanceLevel Level);
    void CheckPerformanceBudget();
    void LogPerformanceWarning(const FString& Warning);
    float CalculateAverageFPS() const;
    float CalculateAverageFrameTime() const;
    void UpdatePerformanceHistory();
};