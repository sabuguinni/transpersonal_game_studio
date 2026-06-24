#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

/**
 * Performance budget tiers for Transpersonal Game Studio
 * Target: 60fps PC (RTX 3070+), 30fps Console (PS5/XSX)
 * Agent #04 — Performance Optimizer
 */

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (PC High-End)"),
    High        UMETA(DisplayName = "High (PC Mid-Range)"),
    Medium      UMETA(DisplayName = "Medium (Console)"),
    Low         UMETA(DisplayName = "Low (PC Low-End)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    /** Distance at which LOD0 transitions to LOD1 (units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD0Distance = 1500.0f;

    /** Distance at which LOD1 transitions to LOD2 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD1Distance = 3000.0f;

    /** Distance at which LOD2 transitions to LOD3 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD2Distance = 6000.0f;

    /** Distance beyond which the mesh is culled entirely */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float CullDistance = 8000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TickBudget
{
    GENERATED_BODY()

    /** Tick interval for hero dinos (< 1500 units from player) in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    float HeroTickInterval = 0.016f; // ~60fps

    /** Tick interval for mid-range dinos (1500-3000 units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    float MidRangeTickInterval = 0.033f; // ~30fps

    /** Tick interval for distant dinos (3000-6000 units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    float DistantTickInterval = 0.1f; // 10fps

    /** Distance beyond which AI tick is paused entirely */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    float AITickPauseDistance = 6000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target frame time in ms (16.6 = 60fps, 33.3 = 30fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TargetFrameTimeMs = 16.6f;

    /** Max GPU time allocated to dinosaur rendering (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float DinoGPUBudgetMs = 4.0f;

    /** Max GPU time allocated to foliage rendering (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float FoliageGPUBudgetMs = 3.0f;

    /** Max GPU time allocated to lighting/shadows (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float LightingGPUBudgetMs = 5.0f;

    /** Max simultaneous skeletal mesh actors in scene */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxSkeletalMeshActors = 20;

    /** Max simultaneous static mesh actors in scene */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxStaticMeshActors = 150;
};

/**
 * UPerf_PerformanceConfig — Data asset holding all performance budgets.
 * Referenced by the PerformanceManager at runtime to enforce frame budgets.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings DinoLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings FoliageLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    FPerf_TickBudget TickBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    /** Apply CVars matching the selected quality tier */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityTierCVars();

    /** Get LOD settings for a given actor based on distance from player */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTickIntervalForDistance(float DistanceFromPlayer) const;
};
