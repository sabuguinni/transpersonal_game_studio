// PerformanceConfig.h — Agent #4 Performance Optimizer
// Cycle: PROD_CYCLE_AUTO_20260626_009
// Target: 60fps PC high-end / 30fps console
// All constants prefixed Perf_ to avoid naming collisions

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

// ============================================================
// ENUMS (global scope — RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Console_30fps   UMETA(DisplayName = "Console 30fps"),
    PC_Medium_60fps UMETA(DisplayName = "PC Medium 60fps"),
    PC_High_60fps   UMETA(DisplayName = "PC High 60fps"),
    PC_Ultra_60fps  UMETA(DisplayName = "PC Ultra 60fps"),
};

UENUM(BlueprintType)
enum class EPerf_LODZone : uint8
{
    Near    UMETA(DisplayName = "Near (0-3000u)"),
    Mid     UMETA(DisplayName = "Mid (3000-5000u)"),
    Far     UMETA(DisplayName = "Far (5000-8000u)"),
    VeryFar UMETA(DisplayName = "VeryFar (8000u+)"),
    Culled  UMETA(DisplayName = "Culled (beyond max draw)"),
};

// ============================================================
// STRUCTS (global scope — RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DinoTickBudget
{
    GENERATED_BODY()

    // Distance from player at which this budget applies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    float MaxDistanceUnits = 3000.0f;

    // Tick interval in seconds for dinos in this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    float TickIntervalSeconds = 0.1f;

    // Whether full AI behavior tree runs at this distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    bool bFullAIEnabled = true;

    // Whether perception system is active at this distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    bool bPerceptionEnabled = true;

    FPerf_DinoTickBudget() {}
    FPerf_DinoTickBudget(float Dist, float Interval, bool bAI, bool bPerc)
        : MaxDistanceUnits(Dist), TickIntervalSeconds(Interval)
        , bFullAIEnabled(bAI), bPerceptionEnabled(bPerc) {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target frame time in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TargetFrameTimeMs = 16.67f; // 60fps

    // Maximum ms allowed for AI systems per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float AIBudgetMs = 3.0f;

    // Maximum ms allowed for rendering per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float RenderBudgetMs = 10.0f;

    // Maximum ms allowed for physics per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float PhysicsBudgetMs = 2.5f;

    // Maximum ms allowed for game logic per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float GameLogicBudgetMs = 1.17f;

    FPerf_FrameBudget() {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODThresholds
{
    GENERATED_BODY()

    // Distance at which LOD0 (full detail) transitions to LOD1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD0_To_LOD1 = 2000.0f;

    // Distance at which LOD1 transitions to LOD2
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD1_To_LOD2 = 4000.0f;

    // Distance at which LOD2 transitions to LOD3 (lowest)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD2_To_LOD3 = 7000.0f;

    // Distance beyond which actor is culled entirely
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float CullDistance = 12000.0f;

    // Dino-specific cull distance (larger = more visible at range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float DinoCullDistance = 15000.0f;

    FPerf_LODThresholds() {}
};

// ============================================================
// MAIN CONFIG CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable, Config = Game)
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    // Current quality tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::PC_High_60fps;

    // Frame budget for this quality tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    // LOD thresholds for meshes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODThresholds LODThresholds;

    // Dino tick budgets by distance zone (Near, Mid, Far, VeryFar)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    TArray<FPerf_DinoTickBudget> DinoTickBudgets;

    // Maximum simultaneous dinos with full AI (beyond this, tick culled)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    int32 MaxFullAIDinos = 8;

    // Maximum total dinos in world (spawner respects this)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Dino")
    int32 MaxTotalDinos = 30;

    // NavMesh query cache radius — reuse patrol points within this radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI")
    float NavMeshCacheRadius = 500.0f;

    // Whether to enable Lumen GI (disable on console tier)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Rendering")
    bool bEnableLumenGI = true;

    // Whether to enable Lumen Reflections
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Rendering")
    bool bEnableLumenReflections = true;

    // Shadow map resolution (1024 console, 2048 PC, 4096 ultra)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Rendering")
    int32 ShadowMapResolution = 2048;

    // Apply this config to the running world via console commands
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyToWorld(UWorld* World);

    // Get the tick interval for a dino at a given distance from player
    UFUNCTION(BlueprintCallable, Category = "Performance|Dino")
    float GetDinoTickInterval(float DistanceFromPlayer) const;

    // Get LOD zone for a given distance
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    EPerf_LODZone GetLODZone(float DistanceFromPlayer) const;

    // Singleton accessor (returns CDO)
    UFUNCTION(BlueprintCallable, Category = "Performance", meta = (WorldContext = "WorldContextObject"))
    static UPerf_PerformanceConfig* Get(UObject* WorldContextObject);
};
