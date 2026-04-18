#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "../SharedTypes.h"
#include "PerformanceMonitor.generated.h"

/**
 * PERFORMANCE OPTIMIZER #04 - PERFORMANCE MONITOR ACTOR
 * 
 * Real-time performance monitoring actor that tracks FPS, memory usage,
 * draw calls, and other critical performance metrics. Provides visual
 * feedback in the world and logs performance data for analysis.
 * 
 * Features:
 * - Real-time FPS monitoring with color-coded visual feedback
 * - Memory usage tracking and alerts
 * - Draw call and triangle count monitoring
 * - Performance tier detection and automatic quality adjustment
 * - Performance logging and data export
 * - Integration with UE5 stat system
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUTime = 0.0f;

    FPerf_FrameData()
    {
        FrameTime = 0.0f;
        FPS = 60.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UsedPhysicalMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UsedVirtualMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TextureMemoryMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AudioMemoryMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StaticMeshMemoryMB = 0.0f;

    FPerf_MemoryData()
    {
        UsedPhysicalMB = 0.0f;
        UsedVirtualMB = 0.0f;
        TextureMemoryMB = 0.0f;
        AudioMemoryMB = 0.0f;
        StaticMeshMemoryMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 Vertices = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TextureMemoryPool = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveLights = 0;

    FPerf_RenderData()
    {
        DrawCalls = 0;
        Triangles = 0;
        Vertices = 0;
        TextureMemoryPool = 0;
        ActiveLights = 0;
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

    // ═══════════════════════════════════════════════════════════════
    // COMPONENTS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* IndicatorMesh;

    // ═══════════════════════════════════════════════════════════════
    // PERFORMANCE MONITORING SETTINGS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableMonitoring = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float UpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableVisualFeedback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float LogInterval = 5.0f;

    // ═══════════════════════════════════════════════════════════════
    // PERFORMANCE TARGETS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MinimumFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxMemoryUsageMB = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxTriangles = 2000000;

    // ═══════════════════════════════════════════════════════════════
    // CURRENT PERFORMANCE DATA
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    FPerf_FrameData CurrentFrameData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    FPerf_MemoryData CurrentMemoryData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    FPerf_RenderData CurrentRenderData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    EEng_PerformanceTier CurrentPerformanceTier;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    bool bIsPerformanceGood = true;

    // ═══════════════════════════════════════════════════════════════
    // PERFORMANCE MONITORING FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceData();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVisualFeedback();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EEng_PerformanceTier DeterminePerformanceTier();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceData();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ExportPerformanceData(const FString& FilePath);

    // ═══════════════════════════════════════════════════════════════
    // PERFORMANCE OPTIMIZATION FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void ApplyPerformanceTier(EEng_PerformanceTier Tier);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeForLowFPS();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeForHighMemory();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void ResetToDefaultSettings();

private:
    // Internal monitoring state
    float LastUpdateTime = 0.0f;
    float LastLogTime = 0.0f;
    TArray<float> FPSHistory;
    TArray<float> MemoryHistory;
    
    // Performance data collection
    void CollectFrameData();
    void CollectMemoryData();
    void CollectRenderData();
    void UpdatePerformanceHistory();
    
    // Visual feedback helpers
    void SetIndicatorColor(const FLinearColor& Color);
    void CreateIndicatorMaterial();
};