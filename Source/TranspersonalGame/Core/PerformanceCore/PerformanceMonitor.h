#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "PerformanceMonitor.generated.h"

DECLARE_STATS_GROUP(TEXT("TranspersonalGame Performance"), STATGROUP_TranspersonalPerf, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Physics Update"), STAT_PhysicsUpdate, STATGROUP_TranspersonalPerf);
DECLARE_CYCLE_STAT(TEXT("AI Processing"), STAT_AIProcessing, STATGROUP_TranspersonalPerf);
DECLARE_CYCLE_STAT(TEXT("Rendering"), STAT_Rendering, STATGROUP_TranspersonalPerf);
DECLARE_CYCLE_STAT(TEXT("Audio Processing"), STAT_AudioProcessing, STATGROUP_TranspersonalPerf);

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Potato      UMETA(DisplayName = "Potato")
};

UENUM(BlueprintType)
enum class EPerf_OptimizationTarget : uint8
{
    FPS_60      UMETA(DisplayName = "60 FPS"),
    FPS_30      UMETA(DisplayName = "30 FPS"),
    FPS_120     UMETA(DisplayName = "120 FPS"),
    Adaptive    UMETA(DisplayName = "Adaptive")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
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
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = FLT_MAX;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        ActiveActors = 0;
        VisibleActors = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerf_PerformanceLevel TargetPerformanceLevel = EPerf_PerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerf_OptimizationTarget TargetFPS = EPerf_OptimizationTarget::FPS_60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAutomaticOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxRenderDistance = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float LODDistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxPhysicsActors = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxAIActors = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableNanite = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLumen = true;

    FPerf_OptimizationSettings()
    {
        TargetPerformanceLevel = EPerf_PerformanceLevel::High;
        TargetFPS = EPerf_OptimizationTarget::FPS_60;
        bEnableAutomaticOptimization = true;
        bEnableLODOptimization = true;
        bEnableOcclusionCulling = true;
        bEnableDistanceCulling = true;
        MaxRenderDistance = 50000.0f;
        LODDistanceMultiplier = 1.0f;
        MaxPhysicsActors = 1000;
        MaxAIActors = 500;
        bEnableNanite = true;
        bEnableLumen = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MetricsUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization = true;

private:
    float MetricsTimer = 0.0f;
    TArray<float> FPSHistory;
    int32 FPSHistoryIndex = 0;
    static const int32 FPSHistorySize = 60;

    float TotalFrameTime = 0.0f;
    int32 FrameCount = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void EnablePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void DisablePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void OptimizeCurrentLevel();

protected:
    void UpdatePerformanceMetrics();
    void ApplyLODOptimizations();
    void ApplyCullingOptimizations();
    void ApplyRenderingOptimizations();
    void ApplyPhysicsOptimizations();
    void LogPerformanceMetrics();
    void CheckPerformanceThresholds();

    float CalculateFrameTime() const;
    int32 GetActiveActorCount() const;
    int32 GetVisibleActorCount() const;
    float GetMemoryUsage() const;
};