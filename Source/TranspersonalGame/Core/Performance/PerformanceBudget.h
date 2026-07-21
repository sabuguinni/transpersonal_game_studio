#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceBudget.generated.h"

/**
 * Performance budget thresholds for Transpersonal Game Studio
 * Target: 60fps PC (RTX 3070+) / 30fps Console (PS5/XSX)
 * Agent #04 — Performance Optimizer
 */

UENUM(BlueprintType)
enum class EPerf_Platform : uint8
{
    PC_High     UMETA(DisplayName = "PC High-End"),
    PC_Mid      UMETA(DisplayName = "PC Mid-Range"),
    Console     UMETA(DisplayName = "Console (PS5/XSX)"),
    Console_Low UMETA(DisplayName = "Console Last-Gen")
};

UENUM(BlueprintType)
enum class EPerf_BudgetStatus : uint8
{
    OK          UMETA(DisplayName = "Within Budget"),
    Warning     UMETA(DisplayName = "Approaching Limit"),
    Critical    UMETA(DisplayName = "Over Budget"),
    Exceeded    UMETA(DisplayName = "Severely Over Budget")
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.67f; // 60fps

    /** Maximum draw calls before LOD kicks in */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    /** Maximum triangle count (millions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxTrianglesM = 8.0f;

    /** Maximum dynamic shadow casters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDynamicShadowCasters = 50;

    /** Maximum active particle systems */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveParticleSystems = 30;

    /** Maximum simultaneous audio sources */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAudioSources = 32;

    /** Texture streaming pool size (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TextureStreamingPoolMB = 1024;

    /** Platform this budget applies to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_Platform Platform = EPerf_Platform::PC_High;
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    /** Distance at which LOD0 transitions to LOD1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0_Distance = 1500.0f;

    /** Distance at which LOD1 transitions to LOD2 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1_Distance = 4000.0f;

    /** Distance at which LOD2 transitions to LOD3 (or culled) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2_Distance = 8000.0f;

    /** Cull distance for small props (rocks, debris) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float SmallPropCullDistance = 3000.0f;

    /** Cull distance for foliage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageCullDistance = 12000.0f;

    /** Cull distance for dinosaurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinosaurCullDistance = 15000.0f;
};

USTRUCT(BlueprintType)
struct FPerf_ShadowBudget
{
    GENERATED_BODY()

    /** Maximum shadow map resolution */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 MaxShadowResolution = 1024;

    /** Maximum CSM cascades */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 MaxCSMCascades = 3;

    /** Shadow distance scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowDistanceScale = 0.8f;

    /** Radius threshold for shadow casting (smaller = fewer shadow casters) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float RadiusThreshold = 0.03f;

    /** Enable contact shadows for characters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    bool bContactShadows = true;

    /** Enable capsule shadows for characters (cheaper than full mesh) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    bool bCapsuleShadows = true;
};

USTRUCT(BlueprintType)
struct FPerf_RuntimeStats
{
    GENERATED_BODY()

    /** Current frame time (ms) */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float CurrentFrameTimeMs = 0.0f;

    /** Current draw call count */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentDrawCalls = 0;

    /** Current actor count in level */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentActorCount = 0;

    /** Current texture streaming pool usage (MB) */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float TexturePoolUsageMB = 0.0f;

    /** Budget status */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    EPerf_BudgetStatus Status = EPerf_BudgetStatus::OK;
};

/**
 * UPerf_BudgetManager — manages performance budgets and applies console var settings
 * Singleton-style UObject, created by PerformanceSubsystem
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_BudgetManager : public UObject
{
    GENERATED_BODY()

public:
    UPerf_BudgetManager();

    /** Apply all performance settings to the current world */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyBudgetSettings(UWorld* World);

    /** Check current frame stats against budget */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_BudgetStatus CheckBudgetStatus(const FPerf_RuntimeStats& Stats) const;

    /** Get LOD settings for a given platform */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_LODSettings GetLODSettings(EPerf_Platform Platform) const;

    /** Get frame budget for a given platform */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget(EPerf_Platform Platform) const;

    /** Apply shadow budget settings via console vars */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyShadowBudget(UWorld* World, const FPerf_ShadowBudget& ShadowBudget);

    /** Log current performance status to output log */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void LogPerformanceStatus() const;

private:
    /** Active frame budget */
    UPROPERTY(VisibleAnywhere, Category = "Performance")
    FPerf_FrameBudget ActiveBudget;

    /** Active LOD settings */
    UPROPERTY(VisibleAnywhere, Category = "Performance")
    FPerf_LODSettings ActiveLOD;

    /** Active shadow budget */
    UPROPERTY(VisibleAnywhere, Category = "Performance")
    FPerf_ShadowBudget ActiveShadow;

    /** Current runtime stats */
    UPROPERTY(VisibleAnywhere, Category = "Performance")
    FPerf_RuntimeStats RuntimeStats;
};
