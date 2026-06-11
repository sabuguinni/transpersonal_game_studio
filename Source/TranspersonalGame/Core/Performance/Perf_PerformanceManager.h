#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Perf_PerformanceManager.generated.h"

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
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFrameTime = 16.67f;
        GPUFrameTime = 8.0f;
        RenderThreadTime = 12.0f;
        GameThreadTime = 10.0f;
        DrawCalls = 1000;
        Triangles = 100000;
        MemoryUsageMB = 512.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 StaticMeshLODBias = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 SkeletalMeshLODBias = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float ShadowDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxParticleCount = 1000;

    FPerf_OptimizationSettings()
    {
        ViewDistanceScale = 1.0f;
        StaticMeshLODBias = 0;
        SkeletalMeshLODBias = 0;
        ShadowDistanceScale = 1.0f;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
        bEnableOcclusionCulling = true;
        MaxParticleCount = 1000;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

public:
    // === PERFORMANCE MONITORING ===
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel TargetPerformanceLevel = EPerf_PerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_OptimizationTarget OptimizationTarget = EPerf_OptimizationTarget::FPS_60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

    // === MONITORING SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float MetricsUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bEnableAutoOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bShowDebugInfo = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float PerformanceThreshold = 30.0f;

    // === PERFORMANCE FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTarget(EPerf_OptimizationTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetToDefaults();

    // === ANALYSIS FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void AnalyzeSceneComplexity();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    TArray<FString> GetOptimizationRecommendations();

protected:
    // === INTERNAL FUNCTIONS ===
    void InitializePerformanceMonitoring();
    void UpdateMetricsInternal();
    void ApplyAutoOptimizations();
    void LogPerformanceData();

    // === TIMER HANDLES ===
    FTimerHandle MetricsUpdateTimer;

    // === PERFORMANCE TRACKING ===
    float LastUpdateTime = 0.0f;
    TArray<float> FrameTimeHistory;
    int32 MaxHistorySize = 60;
};