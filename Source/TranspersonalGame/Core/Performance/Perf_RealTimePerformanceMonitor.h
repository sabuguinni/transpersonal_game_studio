#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Perf_RealTimePerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TimeStamp;

    FPerf_PerformanceSnapshot()
        : CurrentFPS(0.0f)
        , FrameTimeMS(0.0f)
        , GPUTimeMS(0.0f)
        , MemoryUsageMB(0.0f)
        , DrawCalls(0)
        , TriangleCount(0)
        , TimeStamp(0.0f)
    {
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-60 FPS)"),
    Acceptable  UMETA(DisplayName = "Acceptable (30-45 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceLevelChanged, EPerf_PerformanceLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceSnapshot, const FPerf_PerformanceSnapshot&, Snapshot);

/**
 * Real-time performance monitoring subsystem for Transpersonal Game
 * Tracks FPS, frame time, memory usage, and GPU performance
 * Provides automatic quality adjustment based on performance thresholds
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_RealTimePerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_RealTimePerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMonitoring() const { return bIsMonitoring; }

    // Performance data access
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceSnapshot GetCurrentSnapshot() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const { return CurrentPerformanceLevel; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime() const;

    // Performance thresholds
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFPS() const { return TargetFPS; }

    // Automatic quality adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoQualityAdjustment(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsAutoQualityAdjustmentEnabled() const { return bAutoQualityAdjustment; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceLevelChanged OnPerformanceLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceSnapshot OnPerformanceSnapshot;

    // Debug functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void LogCurrentPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ResetPerformanceHistory();

protected:
    // Monitoring state
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel CurrentPerformanceLevel;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "15.0", ClampMax = "120.0"))
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoQualityAdjustment;

    // Monitoring settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "10", ClampMax = "1000"))
    int32 PerformanceHistorySize;

    // Performance history
    UPROPERTY()
    TArray<FPerf_PerformanceSnapshot> PerformanceHistory;

    // Timer handle for monitoring
    FTimerHandle MonitoringTimerHandle;

private:
    // Internal monitoring functions
    void UpdatePerformanceMetrics();
    void AnalyzePerformanceLevel();
    void AdjustQualitySettings();
    
    // Performance calculation helpers
    float CalculateCurrentFPS() const;
    float CalculateFrameTime() const;
    float CalculateGPUTime() const;
    float CalculateMemoryUsage() const;
    int32 CalculateDrawCalls() const;
    int32 CalculateTriangleCount() const;
    
    // Quality adjustment helpers
    void ReduceQualitySettings();
    void IncreaseQualitySettings();
    bool CanReduceQuality() const;
    bool CanIncreaseQuality() const;
    
    // Performance level thresholds
    static constexpr float EXCELLENT_FPS_THRESHOLD = 60.0f;
    static constexpr float GOOD_FPS_THRESHOLD = 45.0f;
    static constexpr float ACCEPTABLE_FPS_THRESHOLD = 30.0f;
    static constexpr float POOR_FPS_THRESHOLD = 15.0f;
    
    // Quality adjustment counters
    int32 QualityAdjustmentCooldown;
    int32 ConsecutivePoorFrames;
    int32 ConsecutiveGoodFrames;
    
    static constexpr int32 QUALITY_ADJUSTMENT_COOLDOWN_FRAMES = 300; // 5 seconds at 60 FPS
    static constexpr int32 POOR_FRAME_THRESHOLD = 180; // 3 seconds at 60 FPS
    static constexpr int32 GOOD_FRAME_THRESHOLD = 600; // 10 seconds at 60 FPS
};