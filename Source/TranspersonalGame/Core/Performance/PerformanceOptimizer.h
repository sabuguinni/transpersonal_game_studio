// PerformanceOptimizer.h
// Agent #04 — Performance Optimizer | Cycle AUTO_20260628_005
// Manages runtime performance budgets, LOD control, and timer-based survival tick
// Target: 60fps PC / 30fps console

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "PerformanceOptimizer.generated.h"

// Forward declarations
class UStaticMeshComponent;
class ADirectionalLight;

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (PC High-End)"),
    High        UMETA(DisplayName = "High (PC Mid-Range)"),
    Medium      UMETA(DisplayName = "Medium (Console)"),
    Low         UMETA(DisplayName = "Low (Performance Mode)")
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target frame time in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.67f; // 60fps

    // GPU budget in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUBudgetMs = 10.0f;

    // CPU budget in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUBudgetMs = 6.0f;

    // Shadow budget (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ShadowBudgetMs = 3.0f;

    // Max draw calls per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    // Max triangles per frame (millions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxTrianglesMillion = 8.0f;
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    // Distance at which LOD0->LOD1 transition occurs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 1500.0f;

    // Distance at which LOD1->LOD2 transition occurs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 4000.0f;

    // Distance at which actors are culled entirely
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 12000.0f;

    // Scale multiplier for LOD distances (1.0 = default)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceScale = 1.5f;

    // Whether to use HLOD (Hierarchical LOD) for distant clusters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bUseHLOD = true;
};

UCLASS(ClassGroup = (Performance), meta = (BlueprintSpawnableComponent), DisplayName = "Performance Optimizer")
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

    // === FRAME BUDGET ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    // === LOD SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings LODSettings;

    // === QUALITY TIER ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    // === SURVIVAL TICK TIMER INTERVAL ===
    // Replaces per-frame Tick for survival stats — fires every N seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Timers", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float SurvivalTickInterval = 0.5f;

    // === SHADOW OPTIMIZATION ===
    // Small props (radius < this value) have shadows disabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows", meta = (ClampMin = "50.0"))
    float SmallPropShadowRadiusThreshold = 150.0f;

    // Max shadow cascade count
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows", meta = (ClampMin = "1", ClampMax = "4"))
    int32 MaxShadowCascades = 3;

    // === RUNTIME STATS (read-only) ===
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats", meta = (AllowPrivateAccess = "true"))
    float CurrentFrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats", meta = (AllowPrivateAccess = "true"))
    int32 CurrentActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats", meta = (AllowPrivateAccess = "true"))
    bool bIsUnderBudget = true;

    // === PUBLIC API ===

    // Apply quality tier settings (console vars + LOD distances)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityTier(EPerf_QualityTier NewTier);

    // Apply LOD settings to all StaticMesh actors in the level
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void ApplyLODSettingsToLevel();

    // Disable shadow casting on small props below radius threshold
    UFUNCTION(BlueprintCallable, Category = "Performance|Shadows")
    void OptimizeSmallPropShadows();

    // Get current frame time in ms
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance|Stats")
    float GetCurrentFrameTimeMs() const { return CurrentFrameTimeMs; }

    // Check if we are within frame budget
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance|Stats")
    bool IsWithinFrameBudget() const { return bIsUnderBudget; }

    // Apply all optimizations for current quality tier
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyAllOptimizations();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Timer handle for survival tick
    FTimerHandle SurvivalTickTimerHandle;

    // Apply console variables for a given quality tier
    void ApplyConsoleVarsForTier(EPerf_QualityTier Tier);

    // Internal frame time tracking
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;
};
