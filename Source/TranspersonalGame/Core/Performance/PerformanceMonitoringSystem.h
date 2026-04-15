#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Stats/Stats.h"
#include "../SharedTypes.h"
#include "PerformanceMonitoringSystem.generated.h"

DECLARE_STATS_GROUP(TEXT("TranspersonalGame"), STATGROUP_TranspersonalGame, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Performance Monitor Update"), STAT_PerformanceMonitorUpdate, STATGROUP_TranspersonalGame);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Actors"), STAT_ActiveActors, STATGROUP_TranspersonalGame);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Frame Time MS"), STAT_FrameTimeMS, STATGROUP_TranspersonalGame);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActorCount;

    FPerf_PerformanceMetrics()
        : FrameTimeMS(0.0f)
        , FPS(0.0f)
        , DrawCalls(0)
        , TriangleCount(0)
        , MemoryUsageMB(0.0f)
        , GPUTimeMS(0.0f)
        , ActiveActorCount(0)
        , VisibleActorCount(0)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFPS_PC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFPS_Console;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxActiveActors;

    FPerf_PerformanceThresholds()
        : TargetFPS_PC(60.0f)
        , TargetFPS_Console(30.0f)
        , MaxFrameTimeMS(16.67f)  // 60 FPS target
        , MaxDrawCalls(2000)
        , MaxMemoryUsageMB(4096.0f)
        , MaxActiveActors(10000)
    {}
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
class TRANSPERSONALGAME_API APerformanceMonitoringSystem : public AActor
{
    GENERATED_BODY()

public:
    APerformanceMonitoringSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Performance monitoring components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    class USceneComponent* RootSceneComponent;

    // Current performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_PerformanceThresholds PerformanceThresholds;

    // Monitoring settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization;

    // Performance history
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<float> FrameTimeHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxHistorySize;

private:
    float TimeSinceLastUpdate;
    float AccumulatedFrameTime;
    int32 FrameCount;

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceHistory();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerPerformanceOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    // Performance optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCullingSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeShadowSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTextureStreaming();

    // Getters for current metrics
    UFUNCTION(BlueprintPure, Category = "Performance")
    const FPerf_PerformanceMetrics& GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    const FPerf_PerformanceThresholds& GetPerformanceThresholds() const { return PerformanceThresholds; }
};