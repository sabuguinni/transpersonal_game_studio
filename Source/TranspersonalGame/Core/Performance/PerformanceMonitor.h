#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "PerformanceMonitor.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-60 FPS)"),
    Fair        UMETA(DisplayName = "Fair (30-45 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
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
    float UsedMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel = EPerf_PerformanceLevel::Good;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        FrameTime = 16.67f;
        GameThreadTime = 8.0f;
        RenderThreadTime = 8.0f;
        GPUTime = 12.0f;
        DrawCalls = 500;
        Triangles = 100000;
        UsedMemoryMB = 512.0f;
        PerformanceLevel = EPerf_PerformanceLevel::Good;
    }
};

USTRUCT(BlueprintType)
struct FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAutoLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxDrawDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float LODBias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxShadowResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bDynamicResolution = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFPS = 60.0f;

    FPerf_OptimizationSettings()
    {
        bEnableAutoLOD = true;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
        bEnableOcclusionCulling = true;
        MaxDrawDistance = 10000.0f;
        LODBias = 0.0f;
        MaxShadowResolution = 2048;
        ViewDistanceScale = 1.0f;
        bDynamicResolution = false;
        TargetFPS = 60.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API APerf_PerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MonitoringInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bDisplayOnScreen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bLogToConsole = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoOptimize = true;

private:
    float TimeSinceLastUpdate = 0.0f;
    TArray<float> FPSHistory;
    int32 MaxHistorySize = 60;
    float TotalFPSSum = 0.0f;
    int32 FrameCount = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const { return CurrentMetrics.PerformanceLevel; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance")
    void OnPerformanceLevelChanged(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance")
    void OnCriticalPerformance(float CurrentFPS);

private:
    void UpdateFPSMetrics(float DeltaTime);
    void UpdateRenderMetrics();
    void UpdateMemoryMetrics();
    void DeterminePerformanceLevel();
    void ApplyAutoOptimizations();
    void DisplayMetricsOnScreen();
};