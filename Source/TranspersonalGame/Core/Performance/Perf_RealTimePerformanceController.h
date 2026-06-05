#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Perf_RealTimePerformanceController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RealTimeMetrics
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
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CullingEfficiency;

    FPerf_RealTimeMetrics()
        : CurrentFPS(60.0f)
        , FrameTimeMS(16.67f)
        , GPUTimeMS(8.0f)
        , MemoryUsageMB(1024.0f)
        , ActiveActorCount(0)
        , VisibleActorCount(0)
        , CullingEfficiency(0.85f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxFrameTimeMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptiveLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDynamicCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableMemoryOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float LODDistanceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float CullingDistanceMultiplier;

    FPerf_OptimizationSettings()
        : TargetFPS(60.0f)
        , MaxFrameTimeMS(16.67f)
        , bEnableAdaptiveLOD(true)
        , bEnableDynamicCulling(true)
        , bEnableMemoryOptimization(true)
        , LODDistanceMultiplier(1.0f)
        , CullingDistanceMultiplier(1.0f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_RealTimePerformanceController : public AActor
{
    GENERATED_BODY()

public:
    APerf_RealTimePerformanceController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    class UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_RealTimeMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MetricsUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization;

private:
    float LastMetricsUpdateTime;
    float PerformanceHistory[60]; // Last 60 frames
    int32 HistoryIndex;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableDynamicCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTerrain();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetOptimizations();

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_RealTimeMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_OptimizationSettings GetOptimizationSettings() const { return OptimizationSettings; }
};