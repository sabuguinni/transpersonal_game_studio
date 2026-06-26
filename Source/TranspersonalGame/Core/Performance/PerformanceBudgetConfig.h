// PerformanceBudgetConfig.h
// Performance Optimizer — Agent #4
// Frame budget constants and LOD thresholds for 60fps PC / 30fps console

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceBudgetConfig.generated.h"

// ============================================================
// Perf_ prefix on all types (RULE 2 — unique names)
// ============================================================

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Console_30fps   UMETA(DisplayName = "Console 30fps"),
    PC_60fps        UMETA(DisplayName = "PC 60fps"),
    PC_High_120fps  UMETA(DisplayName = "PC High 120fps"),
};

USTRUCT(BlueprintType)
struct FPerf_LODThresholds
{
    GENERATED_BODY()

    /** Distance at which LOD0→LOD1 transition occurs (units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD0_To_LOD1 = 2000.f;

    /** Distance at which LOD1→LOD2 transition occurs (units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD1_To_LOD2 = 5000.f;

    /** Distance at which LOD2→cull occurs (units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD2_CullDistance = 10000.f;
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TargetFrameTimeMs = 16.67f; // 60fps

    /** Max skeletal mesh actors in scene before LOD bias kicks in */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxSkeletalActors = 20;

    /** Max dynamic lights before shadow culling activates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxDynamicLights = 8;

    /** Texture streaming pool size in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 TextureStreamingPoolMB = 2048;

    /** Shadow cascade count (CSM) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 ShadowCascades = 3;
};

USTRUCT(BlueprintType)
struct FPerf_DinoLODConfig
{
    GENERATED_BODY()

    /** Species name for identification */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    FName SpeciesName;

    /** LOD thresholds specific to this species */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    FPerf_LODThresholds LODThresholds;

    /** Cull distance — large dinos visible further */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    float CullDistance = 12000.f;

    /** Whether to use animation LOD (reduce bone count at distance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    bool bUseAnimationLOD = true;
};

/**
 * UPerf_BudgetConfig
 * Data asset holding all performance budget constants.
 * Loaded at game startup by the Performance subsystem.
 * Tuned for: 60fps on PC (RTX 3070+), 30fps on console (PS5/XSX).
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_BudgetConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_BudgetConfig();

    /** Active quality tier — set at startup based on hardware detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier ActiveTier = EPerf_QualityTier::PC_60fps;

    /** Frame budget for the active tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_FrameBudget FrameBudget;

    /** Default LOD thresholds for static mesh props */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODThresholds PropLODThresholds;

    /** Per-species dino LOD configurations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    TArray<FPerf_DinoLODConfig> DinoLODConfigs;

    /** Apply console commands matching the active quality tier */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyQualityTierCommands() const;

    /** Get frame budget for a given tier */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetBudgetForTier(EPerf_QualityTier Tier) const;

    /** Returns true if skeletal actor count is within budget */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsSkeletalCountWithinBudget(int32 CurrentCount) const;
};
