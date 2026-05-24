#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_RenderOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    int32 MaxTriangles = 500000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    bool bEnableDynamicBatching = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    bool bEnableInstancing = true;

    FPerf_RenderSettings()
    {
        ViewDistanceScale = 1.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 500000;
        bEnableDynamicBatching = true;
        bEnableInstancing = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Render Metrics")
    int32 CurrentDrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Render Metrics")
    int32 CurrentTriangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Render Metrics")
    float GPUFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Render Metrics")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Render Metrics")
    int32 VisibleActors = 0;

    FPerf_RenderMetrics()
    {
        CurrentDrawCalls = 0;
        CurrentTriangles = 0;
        GPUFrameTime = 0.0f;
        RenderThreadTime = 0.0f;
        VisibleActors = 0;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_RenderOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_RenderOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Render optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Optimization")
    FPerf_RenderSettings RenderSettings;

    // Current render metrics
    UPROPERTY(BlueprintReadOnly, Category = "Render Metrics")
    FPerf_RenderMetrics CurrentMetrics;

    // Optimization controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bAutoOptimizeEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float OptimizationUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float CullingDistanceMultiplier = 1.0f;

    // LOD settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 5000.0f;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void OptimizeRenderingForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetRenderQuality(EPerformanceLevel Quality);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void UpdateCullingDistances();

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void OptimizeStaticMeshLODs();

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    FPerf_RenderMetrics GetCurrentRenderMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetViewDistanceScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void EnableDynamicBatching(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetMaxDrawCalls(int32 MaxCalls);

private:
    // Internal optimization functions
    void UpdateRenderMetrics();
    void ApplyLODOptimizations();
    void OptimizeMaterialComplexity();
    void UpdateInstancedRendering();
    void CheckRenderBudget();

    // Timing
    float LastOptimizationTime = 0.0f;
    float LastMetricsUpdate = 0.0f;

    // Cached references
    TArray<TWeakObjectPtr<AActor>> CachedStaticMeshActors;
    TArray<TWeakObjectPtr<UStaticMeshComponent>> CachedMeshComponents;

    // Performance tracking
    TArray<float> FrameTimeHistory;
    int32 FrameTimeHistoryIndex = 0;
    static constexpr int32 MaxFrameTimeHistory = 60;
};