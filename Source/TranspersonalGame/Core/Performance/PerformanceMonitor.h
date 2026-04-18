#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "../SharedTypes.h"
#include "PerformanceMonitor.generated.h"

DECLARE_STATS_GROUP(TEXT("TranspersonalGame Performance"), STATGROUP_TranspersonalPerformance, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Performance Monitor Update"), STAT_PerformanceMonitorUpdate, STATGROUP_TranspersonalPerformance);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Actors"), STAT_ActiveActors, STATGROUP_TranspersonalPerformance);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Frame Time MS"), STAT_FrameTimeMS, STATGROUP_TranspersonalPerformance);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    FPerf_PerformanceMetrics()
        : FrameTimeMS(0.0f)
        , FPS(0.0f)
        , ActiveActorCount(0)
        , VisibleActorCount(0)
        , GPUTimeMS(0.0f)
        , MemoryUsageMB(0.0f)
        , DrawCalls(0)
        , Triangles(0)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxGPUTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxDrawCalls;

    FPerf_PerformanceThresholds()
        : TargetFPS(60.0f)
        , MaxFrameTimeMS(16.67f)  // 60 FPS = 16.67ms per frame
        , MaxActiveActors(10000)
        , MaxGPUTimeMS(12.0f)
        , MaxMemoryUsageMB(4096.0f)  // 4GB
        , MaxDrawCalls(2000)
    {
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent"),
    Good        UMETA(DisplayName = "Good"),
    Acceptable  UMETA(DisplayName = "Acceptable"),
    Poor        UMETA(DisplayName = "Poor"),
    Critical    UMETA(DisplayName = "Critical")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceThresholds(const FPerf_PerformanceThresholds& NewThresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceThresholds GetPerformanceThresholds() const { return Thresholds; }

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCullingSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeShadowSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTextureSettings();

    // Debug and profiling
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void DisablePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void DumpPerformanceReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_PerformanceThresholds Thresholds;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastUpdateTime;

private:
    FTimerHandle MonitoringTimerHandle;
    
    void CollectFrameMetrics();
    void CollectActorMetrics();
    void CollectGPUMetrics();
    void CollectMemoryMetrics();
    void CheckPerformanceThresholds();
    void ApplyPerformanceOptimizations();
};