#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PerformanceManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceThresholdExceeded, float, CurrentFPS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryThresholdExceeded, float, MemoryUsageMB);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceThresholds
{
    GENERATED_BODY()

    // Target FPS thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_Console = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CriticalFPS_Threshold = 20.0f;

    // Memory thresholds (in MB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsage_PC = 8192.0f; // 8GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsage_Console = 4096.0f; // 4GB

    // Draw call limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    // Triangle count limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTriangles = 2000000; // 2M triangles

    FPerformanceThresholds()
    {
        TargetFPS_PC = 60.0f;
        TargetFPS_Console = 30.0f;
        CriticalFPS_Threshold = 20.0f;
        MaxMemoryUsage_PC = 8192.0f;
        MaxMemoryUsage_Console = 4096.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 2000000;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    FPerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        FrameTime = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        ActiveActors = 0;
        VisibleActors = 0;
    }
};

/**
 * Performance Manager - Core system for monitoring and optimizing game performance
 * Ensures 60fps on PC and 30fps on console through dynamic optimization
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceThresholds(const FPerformanceThresholds& NewThresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForcePerformanceOptimization();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalLODBias(float LODBias);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLODForCurrentPerformance();

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemoryUsage();

    // Dynamic Quality Adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustRenderQuality(float QualityScale);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDynamicQualityEnabled(bool bEnabled);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceThresholdExceeded OnFPSThresholdExceeded;

    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnMemoryThresholdExceeded OnMemoryThresholdExceeded;

protected:
    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyPerformanceOptimizations();

    // Optimization methods
    void OptimizeLighting();
    void OptimizeParticles();
    void OptimizePostProcessing();
    void OptimizeShadows();

private:
    UPROPERTY(EditAnywhere, Category = "Performance")
    FPerformanceThresholds Thresholds;

    UPROPERTY(VisibleAnywhere, Category = "Performance")
    FPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float MonitoringInterval = 0.1f; // Update every 100ms

    UPROPERTY(EditAnywhere, Category = "Performance")
    bool bDynamicQualityEnabled = true;

    UPROPERTY(EditAnywhere, Category = "Performance")
    bool bAutoOptimizationEnabled = true;

    // Internal tracking
    float TimeSinceLastUpdate = 0.0f;
    float PerformanceHistory[10] = {0}; // Rolling average
    int32 HistoryIndex = 0;
    bool bOptimizationInProgress = false;
    
    // Quality scaling factors
    float CurrentQualityScale = 1.0f;
    float TargetQualityScale = 1.0f;
    float QualityTransitionSpeed = 2.0f;
};