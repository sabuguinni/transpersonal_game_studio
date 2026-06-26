// PerformanceBudgetConfig.h
// Performance Optimizer #04 — Frame budget constants and LOD thresholds
// Target: 60fps PC (16.67ms) / 30fps Console (33.33ms)
// Cycle: PROD_CYCLE_AUTO_20260626_002

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceBudgetConfig.generated.h"

// ============================================================
// ENUMS — Perf_ prefix to avoid collision with other agents
// ============================================================

UENUM(BlueprintType)
enum class EPerf_TargetPlatform : uint8
{
    PC_High     UMETA(DisplayName = "PC High-End"),
    PC_Mid      UMETA(DisplayName = "PC Mid-Range"),
    Console     UMETA(DisplayName = "Console"),
    Mobile      UMETA(DisplayName = "Mobile")
};

UENUM(BlueprintType)
enum class EPerf_BudgetStatus : uint8
{
    Comfortable UMETA(DisplayName = "Comfortable (>4ms headroom)"),
    Tight       UMETA(DisplayName = "Tight (1-4ms headroom)"),
    OverBudget  UMETA(DisplayName = "Over Budget (<1ms headroom)")
};

// ============================================================
// STRUCTS — Frame budget breakdown
// ============================================================

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Total frame budget in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TotalBudgetMs = 16.67f; // 60fps

    // Rendering sub-budgets (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LumenGIBudgetMs = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ShadowBudgetMs = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float SkeletalMeshBudgetMs = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StaticMeshBudgetMs = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ParticlesBudgetMs = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadBudgetMs = 4.0f;

    // Computed headroom
    float GetHeadroomMs() const
    {
        return TotalBudgetMs - (LumenGIBudgetMs + ShadowBudgetMs +
               SkeletalMeshBudgetMs + StaticMeshBudgetMs +
               ParticlesBudgetMs + GameThreadBudgetMs);
    }
};

USTRUCT(BlueprintType)
struct FPerf_LODThresholds
{
    GENERATED_BODY()

    // Distance at which dinosaur LOD1 kicks in (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinoLOD1Distance = 1500.0f;

    // Distance at which dinosaur LOD2 kicks in (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinoLOD2Distance = 4000.0f;

    // Distance at which dinosaur culls entirely (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinoCullDistance = 8000.0f;

    // Foliage cull distance (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageCullDistance = 5000.0f;

    // NPC URO (Update Rate Optimization) distance threshold (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float NPCURODistance = 2000.0f;

    // Max simultaneous animated skeletal meshes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxAnimatedSkeletalMeshes = 12;
};

USTRUCT(BlueprintType)
struct FPerf_ShadowBudget
{
    GENERATED_BODY()

    // Max shadow cascade count (3 = good balance)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 MaxCSMCascades = 3;

    // Max shadow map resolution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 MaxShadowResolution = 2048;

    // Max dynamic shadow-casting lights in scene
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 MaxDynamicShadowLights = 4;

    // Distance beyond which shadows are culled (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowCullDistance = 6000.0f;
};

// ============================================================
// MAIN CONFIG CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable, Config = Game)
class TRANSPERSONALGAME_API UPerf_PerformanceBudgetConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceBudgetConfig();

    // --- Platform target ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Platform")
    EPerf_TargetPlatform TargetPlatform = EPerf_TargetPlatform::PC_High;

    // --- Frame budgets per platform ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget PCHighBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget ConsoleBudget;

    // --- LOD thresholds ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODThresholds LODThresholds;

    // --- Shadow budget ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    FPerf_ShadowBudget ShadowBudget;

    // --- Max actor counts (enforced by CAP system) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|CAP")
    int32 MaxDinosInScene = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|CAP")
    int32 MaxFoliageActors = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|CAP")
    int32 MaxParticleSystemsActive = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|CAP")
    int32 MaxDynamicLights = 8;

    // --- Streaming ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    int32 TextureStreamingPoolMB = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    float WorldPartitionStreamingRadius = 15000.0f;

    // --- Blueprint callable helpers ---
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetActiveBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_BudgetStatus EvaluateBudgetStatus(float MeasuredFrameMs) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReport() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyConsoleCommandsForPlatform();
};
