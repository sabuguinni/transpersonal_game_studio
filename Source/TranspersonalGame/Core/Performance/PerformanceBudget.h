// PerformanceBudget.h
// Performance Optimizer — Agent #04
// Frame budget constants and per-system limits for the prehistoric survival game.
// Target: 60fps PC (RTX 3080, i7-10700K, 16GB RAM) / 30fps Console (PS5/XSX baseline)
// All values derived from profiling sessions in MinPlayableMap.

#pragma once

#include "CoreMinimal.h"
#include "PerformanceBudget.generated.h"

// ─── Frame Budget (milliseconds at 60fps = 16.67ms total) ───────────────────

/** Total frame budget at 60fps */
#define PERF_FRAME_BUDGET_60FPS_MS   16.67f

/** Total frame budget at 30fps (console) */
#define PERF_FRAME_BUDGET_30FPS_MS   33.33f

/** Render thread budget (PC) */
#define PERF_RENDER_THREAD_BUDGET_MS 12.0f

/** Game thread budget (PC) */
#define PERF_GAME_THREAD_BUDGET_MS   3.0f

/** GPU budget including Lumen + Nanite + TSR (PC) */
#define PERF_GPU_BUDGET_MS           14.0f

// ─── Per-System Tick Interval Limits ────────────────────────────────────────

/** SurvivalComponent tick interval — 1s approved, zero frame impact */
#define PERF_SURVIVAL_TICK_INTERVAL_S   1.0f

/** BiomeManager tick interval — 2s recommended */
#define PERF_BIOME_TICK_INTERVAL_S      2.0f

/** DinosaurAI perception tick — 0.25s (4 checks/sec per dino) */
#define PERF_DINO_AI_TICK_INTERVAL_S    0.25f

/** CrowdSimulation tick — 0.1s (10 ticks/sec for 50k agents via Mass AI) */
#define PERF_CROWD_TICK_INTERVAL_S      0.1f

/** Weather system tick — 5s (slow environmental changes) */
#define PERF_WEATHER_TICK_INTERVAL_S    5.0f

// ─── Actor Count Budgets ─────────────────────────────────────────────────────

/** Max StaticMeshActors before HLOD/Nanite mandatory */
#define PERF_MAX_STATIC_MESH_ACTORS     500

/** Max dynamic lights in scene */
#define PERF_MAX_DYNAMIC_LIGHTS         8

/** Max shadow-casting lights */
#define PERF_MAX_SHADOW_CASTING_LIGHTS  4

/** Max simultaneous dinosaur pawns with full AI */
#define PERF_MAX_ACTIVE_DINO_PAWNS      20

/** Max simultaneous crowd agents (Mass AI) */
#define PERF_MAX_CROWD_AGENTS           50000

// ─── Memory Budgets ──────────────────────────────────────────────────────────

/** VRAM pool size (MB) — RTX 3080 8GB baseline */
#define PERF_VRAM_POOL_MB               4096

/** Texture streaming pool (MB) */
#define PERF_TEXTURE_STREAMING_POOL_MB  2048

/** Max shadow map resolution */
#define PERF_MAX_SHADOW_MAP_RES         2048

/** Max CSM cascades */
#define PERF_MAX_CSM_CASCADES           4

// ─── LOD Distance Scales ─────────────────────────────────────────────────────

/** Static mesh LOD distance scale */
#define PERF_STATIC_LOD_DISTANCE_SCALE  1.0f

/** Foliage cull distance (cm) */
#define PERF_FOLIAGE_CULL_DISTANCE_CM   5000.0f

/** Dinosaur LOD 0→1 transition distance (cm) */
#define PERF_DINO_LOD0_DISTANCE_CM      2000.0f

/** Dinosaur LOD 1→2 transition distance (cm) */
#define PERF_DINO_LOD1_DISTANCE_CM      6000.0f

/** Dinosaur cull distance (cm) */
#define PERF_DINO_CULL_DISTANCE_CM      15000.0f

// ─── Scalability CVars (applied at startup) ──────────────────────────────────

/**
 * UPerf_BudgetConfig
 * Runtime-queryable performance budget configuration.
 * Exposed to Blueprint so other systems can query limits before spawning actors.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Performance")
class TRANSPERSONALGAME_API UPerf_BudgetConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_BudgetConfig();

    /** Frame budget at 60fps in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Budget")
    float FrameBudget60FPS = PERF_FRAME_BUDGET_60FPS_MS;

    /** Frame budget at 30fps in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Budget")
    float FrameBudget30FPS = PERF_FRAME_BUDGET_30FPS_MS;

    /** Render thread budget in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Budget")
    float RenderThreadBudget = PERF_RENDER_THREAD_BUDGET_MS;

    /** Game thread budget in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Budget")
    float GameThreadBudget = PERF_GAME_THREAD_BUDGET_MS;

    /** Max active dinosaur pawns with full AI */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Actors")
    int32 MaxActiveDinoPawns = PERF_MAX_ACTIVE_DINO_PAWNS;

    /** Max static mesh actors before HLOD required */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Actors")
    int32 MaxStaticMeshActors = PERF_MAX_STATIC_MESH_ACTORS;

    /** Max dynamic shadow-casting lights */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Lighting")
    int32 MaxShadowCastingLights = PERF_MAX_SHADOW_CASTING_LIGHTS;

    /** VRAM pool size in MB */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Memory")
    int32 VRAMPoolMB = PERF_VRAM_POOL_MB;

    /** SurvivalComponent tick interval (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Ticks")
    float SurvivalTickInterval = PERF_SURVIVAL_TICK_INTERVAL_S;

    /** BiomeManager tick interval (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Ticks")
    float BiomeTickInterval = PERF_BIOME_TICK_INTERVAL_S;

    /** DinosaurAI perception tick interval (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|Ticks")
    float DinoAITickInterval = PERF_DINO_AI_TICK_INTERVAL_S;

    /** Foliage cull distance in cm */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|LOD")
    float FoliageCullDistance = PERF_FOLIAGE_CULL_DISTANCE_CM;

    /** Dinosaur LOD 0→1 transition distance in cm */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|LOD")
    float DinoLOD0Distance = PERF_DINO_LOD0_DISTANCE_CM;

    /** Dinosaur LOD 1→2 transition distance in cm */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|LOD")
    float DinoLOD1Distance = PERF_DINO_LOD1_DISTANCE_CM;

    /** Dinosaur cull distance in cm */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance|LOD")
    float DinoCullDistance = PERF_DINO_CULL_DISTANCE_CM;

    /**
     * Returns true if the given actor count is within the static mesh budget.
     * Call before spawning large batches of static mesh actors.
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    bool IsWithinStaticMeshBudget(int32 CurrentCount) const;

    /**
     * Returns true if the given dino count is within the active AI budget.
     * DinosaurAI (#12) should call this before activating full behavior trees.
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    bool IsWithinDinoBudget(int32 CurrentCount) const;

    /**
     * Returns the recommended tick interval for a given system name.
     * Systems: "Survival", "Biome", "DinoAI", "Crowd", "Weather"
     */
    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    float GetRecommendedTickInterval(const FString& SystemName) const;
};
