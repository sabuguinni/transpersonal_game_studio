#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Perf_PerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra      UMETA(DisplayName = "Ultra"),
    High       UMETA(DisplayName = "High"), 
    Medium     UMETA(DisplayName = "Medium"),
    Low        UMETA(DisplayName = "Low"),
    Potato     UMETA(DisplayName = "Potato")
};

UENUM(BlueprintType)
enum class EPerf_OptimizationTarget : uint8
{
    FPS_60     UMETA(DisplayName = "60 FPS"),
    FPS_30     UMETA(DisplayName = "30 FPS"),
    Memory     UMETA(DisplayName = "Memory"),
    Battery    UMETA(DisplayName = "Battery Life")
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
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFrameTime = 16.67f;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        PhysicsTime = 0.0f;
        RenderTime = 0.0f;
        GameThreadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 ShadowMapResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    int32 TexturePoolSizeMB = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsSubstepDelta = 0.016667f;

    FPerf_OptimizationSettings()
    {
        LODDistanceScale = 1.0f;
        MaxLODLevel = 3;
        ShadowMapResolution = 2048;
        ShadowDistanceScale = 1.0f;
        CullDistanceScale = 1.0f;
        ViewDistanceScale = 1.0f;
        TexturePoolSizeMB = 2000;
        bEnablePhysicsOptimization = true;
        PhysicsSubstepDelta = 0.016667f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceOptimizer : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // === PERFORMANCE MONITORING ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    bool bEnablePerformanceMonitoring = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float MonitoringInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    EPerf_PerformanceLevel TargetPerformanceLevel = EPerf_PerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    EPerf_OptimizationTarget OptimizationTarget = EPerf_OptimizationTarget::FPS_60;

    // === PERFORMANCE METRICS ===
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PerformanceMetrics AverageMetrics;

    // === OPTIMIZATION SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bAutoOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float AutoOptimizationThreshold = 45.0f; // FPS threshold for auto-optimization

    // === PERFORMANCE FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationSettings(const FPerf_OptimizationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTarget(EPerf_OptimizationTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeShadowSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTextureSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsSettings();

    // === PROFILING FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Profiling", CallInEditor)
    void StartMemoryProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling", CallInEditor)
    void StartFPSProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling", CallInEditor)
    void StartRenderingProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling", CallInEditor)
    void StartPhysicsProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling", CallInEditor)
    void StopAllProfiling();

    // === OPTIMIZATION PRESETS ===
    UFUNCTION(BlueprintCallable, Category = "Optimization Presets")
    void ApplyUltraSettings();

    UFUNCTION(BlueprintCallable, Category = "Optimization Presets")
    void ApplyHighSettings();

    UFUNCTION(BlueprintCallable, Category = "Optimization Presets")
    void ApplyMediumSettings();

    UFUNCTION(BlueprintCallable, Category = "Optimization Presets")
    void ApplyLowSettings();

    UFUNCTION(BlueprintCallable, Category = "Optimization Presets")
    void ApplyPotatoSettings();

protected:
    // === INTERNAL MONITORING ===
    UPROPERTY()
    FTimerHandle MonitoringTimer;

    UPROPERTY()
    TArray<FPerf_PerformanceMetrics> MetricsHistory;

    UPROPERTY()
    int32 MaxHistorySize = 60; // 1 minute of history at 1Hz

    // === INTERNAL FUNCTIONS ===
    UFUNCTION()
    void UpdatePerformanceMetrics();

    UFUNCTION()
    void CheckAutoOptimization();

    void CalculateAverageMetrics();
    void ApplyConsoleCommands();
    void LogPerformanceWarning(const FString& Warning);

private:
    bool bIsMonitoring = false;
    float LastFrameTime = 0.0f;
    int32 FrameCounter = 0;
};