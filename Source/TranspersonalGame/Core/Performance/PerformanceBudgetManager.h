// PerformanceBudgetManager.h — Performance Optimizer Agent #4
// Target: 60fps PC / 30fps console
// Manages LOD distances, draw call budgets, tick rate throttling
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "PerformanceBudgetManager.generated.h"

// LOD distance preset enum — unique prefix Perf_
UENUM(BlueprintType)
enum class EPerf_LODPreset : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (PC High-End)"),
    High        UMETA(DisplayName = "High (PC Mid-Range)"),
    Medium      UMETA(DisplayName = "Medium (Console)"),
    Low         UMETA(DisplayName = "Low (Console Portable)"),
};

// Performance tier for dynamic scaling
UENUM(BlueprintType)
enum class EPerf_PerformanceTier : uint8
{
    Green       UMETA(DisplayName = "Green — 60fps+"),
    Yellow      UMETA(DisplayName = "Yellow — 45-60fps"),
    Red         UMETA(DisplayName = "Red — Below 45fps"),
};

// LOD distance table row
USTRUCT(BlueprintType)
struct FPerf_LODDistanceRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0_Distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1_Distance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2_Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD3_Distance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 10000.0f;
};

// Per-frame performance snapshot
USTRUCT(BlueprintType)
struct FPerf_FrameSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCallCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceTier Tier = EPerf_PerformanceTier::Green;
};

// Budget limits per platform
USTRUCT(BlueprintType)
struct FPerf_BudgetLimits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxDrawCalls = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxDynamicLights = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxShadowCastingLights = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxFrameTimeMs = 16.67f; // 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float TextureStreamingPoolMB = 1024.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxActiveActors = 500;
};

UCLASS(ClassGroup = (Performance), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API APerformanceBudgetManager : public AActor
{
    GENERATED_BODY()

public:
    APerformanceBudgetManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Current LOD preset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    EPerf_LODPreset CurrentLODPreset;

    // Current performance tier (updated each frame)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceTier CurrentTier;

    // Budget limits for current platform
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_BudgetLimits BudgetLimits;

    // Latest frame snapshot
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FrameSnapshot LatestSnapshot;

    // LOD distance scale multiplier (1.0 = default)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float LODDistanceScale;

    // Enable dynamic quality scaling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDynamicScaling;

    // Screen percentage (100 = full res, 75 = 75% res)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "50", ClampMax = "100"))
    int32 ScreenPercentage;

    // Tick interval for performance checks (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float PerformanceCheckInterval;

    // History of frame times for rolling average
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<float> FrameTimeHistory;

    // Max history samples
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxHistorySamples;

public:
    // Apply LOD preset to all actors in world
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void ApplyLODPreset(EPerf_LODPreset Preset);

    // Get LOD distances for a given preset
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    FPerf_LODDistanceRow GetLODDistances(EPerf_LODPreset Preset) const;

    // Update performance snapshot
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceSnapshot(float DeltaTime);

    // Get rolling average frame time
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTimeMs() const;

    // Get current FPS estimate
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetEstimatedFPS() const;

    // Evaluate tier from frame time
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceTier EvaluatePerformanceTier(float FrameTimeMs) const;

    // Apply dynamic quality reduction when RED tier
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyDynamicQualityReduction();

    // Restore quality when GREEN tier
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RestoreQuality();

    // Apply all performance console variables
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyPerformanceCVars();

    // Optimize all lights in scene (disable shadow on non-critical)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void OptimizeSceneLights();

    // Set cull distances on all static meshes
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void SetCullDistancesOnMeshes(float CullDistance);

    // Log full performance report to output log
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void LogPerformanceReport() const;

private:
    float TimeSinceLastCheck;
    bool bQualityReduced;

    void InitializeLODTables();
    TMap<EPerf_LODPreset, FPerf_LODDistanceRow> LODTable;
};
