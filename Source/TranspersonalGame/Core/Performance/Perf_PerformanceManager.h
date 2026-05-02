#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Perf_PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        FrameTime = 0.0f;
        DrawCalls = 0;
        MemoryUsageMB = 0.0f;
        VisibleActors = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float ViewDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageLODScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 SkeletalMeshLODBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxShadowResolution;

    FPerf_LODSettings()
    {
        ViewDistanceScale = 1.0f;
        FoliageLODScale = 1.0f;
        SkeletalMeshLODBias = 0;
        MaxShadowResolution = 4096;
    }
};

/**
 * Performance Manager Subsystem
 * Monitors and optimizes game performance to maintain 60fps on PC and 30fps on console
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODSettings(const FPerf_LODSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_LODSettings GetCurrentLODSettings() const;

    // Dynamic optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForCurrentConditions();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMonitoring(bool bEnabled);

    // Performance targets
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFPS() const;

protected:
    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void AdjustQualitySettings();

    // LOD optimization
    void OptimizeLODSettings();
    void UpdateViewDistanceScaling();
    void OptimizeShadowSettings();

private:
    UPROPERTY()
    EPerf_PerformanceLevel CurrentPerformanceLevel;

    UPROPERTY()
    FPerf_PerformanceMetrics CachedMetrics;

    UPROPERTY()
    FPerf_LODSettings CurrentLODSettings;

    UPROPERTY()
    float TargetFPS;

    UPROPERTY()
    bool bPerformanceMonitoringEnabled;

    UPROPERTY()
    float LastOptimizationTime;

    // Performance thresholds
    static constexpr float LOW_FPS_THRESHOLD = 25.0f;
    static constexpr float HIGH_FPS_THRESHOLD = 55.0f;
    static constexpr float OPTIMIZATION_INTERVAL = 5.0f;
};