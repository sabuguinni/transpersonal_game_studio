// PerformanceBudgetManager.h
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260624_006
// Manages per-frame draw call budgets, LOD distances, and shadow cull rules
// for 60fps PC / 30fps console targets.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerformanceBudgetManager.generated.h"

// ── Perf-unique types (prefix Perf_ to avoid collisions) ──────────────────

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Console_30fps   UMETA(DisplayName = "Console 30fps"),
    PC_Medium_60fps UMETA(DisplayName = "PC Medium 60fps"),
    PC_High_60fps   UMETA(DisplayName = "PC High 60fps"),
    PC_Ultra_60fps  UMETA(DisplayName = "PC Ultra 60fps"),
};

USTRUCT(BlueprintType)
struct FPerf_DrawCallBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSkeletalDrawCalls = 400;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxStaticDrawCalls = 1200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxParticleDrawCalls = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.67f; // 60fps
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float NearCullDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MidCullDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FarCullDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float SkeletalLODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageLODDistanceScale = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bDisableShadowsOnSmallProps = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float SmallPropScaleThreshold = 0.5f;
};

// ── Main manager actor ─────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Performance Budget Manager"))
class TRANSPERSONALGAME_API APerformanceBudgetManager : public AActor
{
    GENERATED_BODY()

public:
    APerformanceBudgetManager();

    // ── Quality tier ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::PC_High_60fps;

    // ── Draw call budget per tier ─────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_DrawCallBudget DrawCallBudget;

    // ── LOD settings ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings LODSettings;

    // ── Runtime stats (read-only) ─────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    int32 EstimatedDrawCalls = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    int32 SkeletalMeshActorCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    int32 StaticMeshActorCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    bool bBudgetExceeded = false;

    // ── Blueprint-callable API ────────────────────────────────────────────

    /** Apply LOD cull distances to all static mesh actors in the level */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyLODCullDistances();

    /** Disable shadow casting on props smaller than SmallPropScaleThreshold */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void DisableShadowsOnSmallProps();

    /** Estimate current draw call count and update bBudgetExceeded */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void AuditDrawCallBudget();

    /** Apply quality tier presets (LOD distances, shadow quality, etc.) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityTierPreset(EPerf_QualityTier Tier);

    /** Returns true if the scene is within the draw call budget */
    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsWithinBudget() const { return !bBudgetExceeded; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float BudgetCheckInterval = 2.0f;
    float TimeSinceLastCheck = 0.0f;
};
