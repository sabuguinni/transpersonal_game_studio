#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Perf_OptimizationManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_OptimizationLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

USTRUCT(BlueprintType)
struct FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxLODLevel = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadow Settings")
    int32 ShadowResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    FPerf_OptimizationSettings()
    {
        CullingDistance = 5000.0f;
        MaxLODLevel = 3;
        ShadowResolution = 2048;
        bEnableOcclusion = true;
        TargetFrameRate = 60.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 VisibleActors = 0;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFrameTime = 0.0f;
        DrawCalls = 0;
        MemoryUsageMB = 0.0f;
        VisibleActors = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_OptimizationManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_OptimizationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerf_OptimizationLevel OptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float MonitoringUpdateInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Optimization")
    TArray<AActor*> OptimizedActors;

private:
    float LastMonitoringUpdate;
    float FrameTimeAccumulator;
    int32 FrameCount;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationLevel(EPerf_OptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeAllStaticMeshes();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCullingDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableLODSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceCounters();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void OptimizeSceneForPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ValidatePerformanceTargets();

protected:
    void UpdatePerformanceMetrics(float DeltaTime);
    void ApplyLODSettings();
    void ApplyCullingSettings();
    void ApplyShadowSettings();
    void OptimizeStaticMeshComponent(UStaticMeshComponent* MeshComponent);
};