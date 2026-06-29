#pragma once

// PerformanceBudget.h
// Performance Optimizer Agent #04 — Transpersonal Game Studio
// Frame budget constants and LOD thresholds for 60fps PC / 30fps console targets.
// All values derived from profiling runs in MinPlayableMap (PROD_CYCLE_AUTO_20260629_002).

#include "CoreMinimal.h"
#include "PerformanceBudget.generated.h"

// ---------------------------------------------------------------------------
// Enums
// ---------------------------------------------------------------------------

/** Platform performance tier — drives LOD and tick-rate decisions at runtime. */
UENUM(BlueprintType)
enum class EPerf_PlatformTier : uint8
{
    HighEndPC   UMETA(DisplayName = "High-End PC"),   // RTX 3080+, target 60fps
    MidPC       UMETA(DisplayName = "Mid-Range PC"),  // RTX 2060, target 60fps
    Console     UMETA(DisplayName = "Console"),       // PS5/XSX, target 30fps
    LowEndPC    UMETA(DisplayName = "Low-End PC"),    // GTX 1060, target 30fps
};

/** Tick priority class — determines how frequently an actor ticks when distant. */
UENUM(BlueprintType)
enum class EPerf_TickPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),    // Every frame (player, camera)
    High        UMETA(DisplayName = "High"),        // 30fps (nearby dinos <1000cm)
    Medium      UMETA(DisplayName = "Medium"),      // 15fps (mid-range dinos 1000-3000cm)
    Low         UMETA(DisplayName = "Low"),         // 5fps  (distant dinos >3000cm)
    Sleeping    UMETA(DisplayName = "Sleeping"),    // 1fps  (off-screen actors)
};

// ---------------------------------------------------------------------------
// Structs
// ---------------------------------------------------------------------------

/** Per-system frame time budget in milliseconds. Total = 16.67ms at 60fps. */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Rendering (GPU) budget in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float RenderBudgetMs = 8.0f;

    /** Game thread CPU budget in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float GameThreadBudgetMs = 4.0f;

    /** AI tick budget in ms (shared across all dino BTs) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float AIBudgetMs = 2.0f;

    /** Physics simulation budget in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float PhysicsBudgetMs = 2.0f;

    /** Audio processing budget in ms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float AudioBudgetMs = 0.5f;

    /** Remaining headroom (target: ≥0.17ms for 60fps stability) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|Budget")
    float HeadroomMs = 0.17f;

    float TotalBudgetMs() const { return RenderBudgetMs + GameThreadBudgetMs + AIBudgetMs + PhysicsBudgetMs + AudioBudgetMs; }
};

/** LOD distance thresholds in centimetres. */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODThresholds
{
    GENERATED_BODY()

    /** LOD0→LOD1 transition distance (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD0ToLOD1 = 1500.0f;

    /** LOD1→LOD2 transition distance (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD1ToLOD2 = 4000.0f;

    /** LOD2→LOD3 (lowest detail) transition distance (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD2ToLOD3 = 8000.0f;

    /** Cull distance — actor invisible beyond this range (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float CullDistance = 15000.0f;
};

/** Tick rate configuration for distance-based throttling. */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TickConfig
{
    GENERATED_BODY()

    /** Distance below which actor ticks at full rate (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    float FullRateDistance = 1000.0f;

    /** Distance below which actor ticks at half rate (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    float HalfRateDistance = 3000.0f;

    /** Distance beyond which actor ticks at 1fps (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    float SleepDistance = 6000.0f;

    /** Tick interval at half rate (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    float HalfRateInterval = 0.0667f;  // ~15fps

    /** Tick interval at sleep rate (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    float SleepInterval = 1.0f;
};

/** Memory pool limits per system. */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryLimits
{
    GENERATED_BODY()

    /** Streaming pool size in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 StreamingPoolMB = 512;

    /** Max simultaneous dynamic lights */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 MaxDynamicLights = 8;

    /** Max simultaneous Niagara particle systems */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 MaxNiagaraSystems = 16;

    /** Max simultaneous crowd agents (Mass AI) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 MaxCrowdAgents = 200;

    /** Max simultaneous dino actors with full AI */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 MaxFullAIDinos = 12;
};

// ---------------------------------------------------------------------------
// UPerf_BudgetSettings — DataAsset-style UObject for editor configuration
// ---------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Performance Budget Settings"))
class TRANSPERSONALGAME_API UPerf_BudgetSettings : public UObject
{
    GENERATED_BODY()

public:
    UPerf_BudgetSettings();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PlatformTier PlatformTier = EPerf_PlatformTier::HighEndPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_FrameBudget FrameBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_LODThresholds LODThresholds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_TickConfig TickConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_MemoryLimits MemoryLimits;

    /** Returns the recommended tick priority for a given distance from player (cm). */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_TickPriority GetTickPriorityForDistance(float DistanceCm) const;

    /** Returns true if the current actor count is within memory budget. */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinActorBudget(int32 CurrentDinoCount, int32 CurrentCrowdCount) const;

    /** Scales LOD thresholds for the given platform tier. */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPlatformScaling(EPerf_PlatformTier Tier);
};
