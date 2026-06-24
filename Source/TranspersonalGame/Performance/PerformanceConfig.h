// PerformanceConfig.h
// Performance Optimizer #04 — PROD_CYCLE_AUTO_20260624_008
// Defines performance budgets, LOD thresholds, and scalability targets.
// All types prefixed with Perf_ to avoid collisions (RULE 2).

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

// ── ENUMS ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_TargetPlatform : uint8
{
    PC_High     UMETA(DisplayName = "PC High-End (60fps)"),
    PC_Mid      UMETA(DisplayName = "PC Mid-Range (45fps)"),
    Console     UMETA(DisplayName = "Console (30fps)"),
    Mobile      UMETA(DisplayName = "Mobile (30fps)")
};

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD0_Full       UMETA(DisplayName = "LOD0 — Full Detail (<1500cm)"),
    LOD1_Half       UMETA(DisplayName = "LOD1 — 50% Tris (1500-4000cm)"),
    LOD2_Impostor   UMETA(DisplayName = "LOD2 — Impostor (>4000cm)"),
    LOD3_Culled     UMETA(DisplayName = "LOD3 — Culled (>8000cm)")
};

UENUM(BlueprintType)
enum class EPerf_DinoSizeClass : uint8
{
    Large   UMETA(DisplayName = "Large (T-Rex, Brachiosaurus) — cull 8000cm"),
    Medium  UMETA(DisplayName = "Medium (Triceratops) — cull 6000cm"),
    Small   UMETA(DisplayName = "Small (Raptor, Protoceratops) — cull 5000cm")
};

// ── STRUCTS ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_LODThresholds
{
    GENERATED_BODY()

    /** Distance (cm) at which LOD0→LOD1 transition occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD0_To_LOD1 = 1500.0f;

    /** Distance (cm) at which LOD1→LOD2 transition occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD1_To_LOD2 = 4000.0f;

    /** Distance (cm) at which LOD2→Culled transition occurs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD2_To_Culled = 8000.0f;
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target platform for this budget */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    EPerf_TargetPlatform Platform = EPerf_TargetPlatform::PC_High;

    /** Target frames per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TargetFPS = 60.0f;

    /** Max frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MaxFrameTimeMS = 16.67f;

    /** Max simultaneous active AI controllers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxActiveAIControllers = 20;

    /** Max simultaneous skeletal mesh actors with full animation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxAnimatedSkeletalMeshes = 20;

    /** Max dynamic shadow casting lights */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxDynamicShadowLights = 4;

    /** Texture streaming pool size in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 TextureStreamingPoolMB = 1024;
};

USTRUCT(BlueprintType)
struct FPerf_DinoLODConfig
{
    GENERATED_BODY()

    /** Size class determines cull distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    EPerf_DinoSizeClass SizeClass = EPerf_DinoSizeClass::Medium;

    /** LOD transition distances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    FPerf_LODThresholds LODThresholds;

    /** Enable animation Update Rate Optimization (URO) for distant dinos */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    bool bEnableURO = true;

    /** URO max interpolation frames (higher = more performance, less smooth) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    int32 URO_MaxInterpolation = 4;

    /** Disable cloth simulation (no cloth on dinos) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    bool bDisableCloth = true;
};

// ── PERFORMANCE MANAGER UOBJECT ───────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    // ── Frame Budgets ──────────────────────────────────────────────────────

    /** PC High-End budget (60fps target) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budgets")
    FPerf_FrameBudget PCHighBudget;

    /** Console budget (30fps target) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budgets")
    FPerf_FrameBudget ConsoleBudget;

    // ── LOD Configuration ─────────────────────────────────────────────────

    /** LOD config for large dinosaurs (T-Rex, Brachiosaurus) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_DinoLODConfig LargeDinoLOD;

    /** LOD config for medium dinosaurs (Triceratops, Ankylosaurus) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_DinoLODConfig MediumDinoLOD;

    /** LOD config for small dinosaurs (Raptor, Protoceratops) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_DinoLODConfig SmallDinoLOD;

    // ── Scalability CVars ─────────────────────────────────────────────────

    /** Shadow quality (0-3) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|CVars", meta = (ClampMin = "0", ClampMax = "3"))
    int32 ShadowQuality = 3;

    /** Volumetric fog grid pixel size (larger = faster, less detail) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|CVars", meta = (ClampMin = "4", ClampMax = "16"))
    int32 VolumetricFogGridPixelSize = 8;

    /** Foliage density scale (1.0 = full, 0.5 = half) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|CVars", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float FoliageDensityScale = 1.0f;

    /** Grass density scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|CVars", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float GrassDensityScale = 1.0f;

    // ── Methods ───────────────────────────────────────────────────────────

    /** Apply PC High-End settings via console commands */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPCHighSettings();

    /** Apply Console settings via console commands */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyConsoleSettings();

    /** Get the LOD config for a given dino size class */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_DinoLODConfig GetDinoLODConfig(EPerf_DinoSizeClass SizeClass) const;

    /** Check if current actor count is within budget */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinActorBudget(int32 CurrentActorCount, EPerf_TargetPlatform Platform) const;
};
