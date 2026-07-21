#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerformanceManager.generated.h"

// AI LOD tier — controls how much CPU an AI agent consumes based on distance
UENUM(BlueprintType)
enum class EPerf_AILODTier : uint8
{
    Full        UMETA(DisplayName = "Full (0-50m)"),
    Reduced     UMETA(DisplayName = "Reduced (50-150m)"),
    Dormant     UMETA(DisplayName = "Dormant (150m+)")
};

// Per-actor performance snapshot
USTRUCT(BlueprintType)
struct FPerf_ActorBudget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DistanceToPlayer = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_AILODTier AILODTier = EPerf_AILODTier::Full;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bTickEnabled = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TickInterval = 0.1f;
};

// Frame budget tracking
USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target FPS (60 PC, 30 console)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.f;

    // Current measured FPS (updated each second)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.f;

    // Frame time in ms
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs = 0.f;

    // Number of active AI agents this frame
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveAICount = 0;

    // Number of dormant AI agents
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DormantAICount = 0;

    // Whether we are below target FPS
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bBelowTargetFPS = false;
};

/**
 * APerf_PerformanceManager
 *
 * Manages AI LOD tiers, tick intervals, and frame budget for the prehistoric survival game.
 * Placed once in the level — automatically adjusts all registered AI actors.
 *
 * Budget targets:
 *   PC high-end:  60fps (16.67ms frame budget)
 *   Console:      30fps (33.33ms frame budget)
 *
 * AI LOD distances:
 *   0-50m:    Full BT + perception (full tick cost)
 *   50-150m:  Reduced BT (tick interval x5)
 *   150m+:    Dormant (tick interval x20, no perception)
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Performance Manager"))
class TRANSPERSONALGAME_API APerf_PerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Configuration ──────────────────────────────────────────────────────

    // Distance at which AI switches from Full to Reduced tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI LOD")
    float AILODFullDistance = 5000.f;  // 50m in UE units (1 unit = 1cm)

    // Distance at which AI switches from Reduced to Dormant tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI LOD")
    float AILODDormantDistance = 15000.f;  // 150m

    // Tick interval for Reduced tier (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI LOD")
    float ReducedTickInterval = 0.5f;

    // Tick interval for Dormant tier (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI LOD")
    float DormantTickInterval = 2.0f;

    // How often (seconds) to re-evaluate AI LOD tiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI LOD")
    float LODUpdateInterval = 1.0f;

    // Target FPS for budget enforcement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TargetFPS = 60.f;

    // ── Runtime State ──────────────────────────────────────────────────────

    // Current frame budget snapshot
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    // ── Public API ─────────────────────────────────────────────────────────

    // Register an actor for LOD management (call from BeginPlay of AI actors)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterAIActor(AActor* Actor);

    // Unregister an actor (call from EndPlay)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterAIActor(AActor* Actor);

    // Get the current LOD tier for a registered actor
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_AILODTier GetActorLODTier(AActor* Actor) const;

    // Force immediate LOD update for all registered actors
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ForceUpdateAllLOD();

    // Get singleton instance (set in BeginPlay)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    static APerf_PerformanceManager* GetInstance();

    // Apply recommended console vars for 60fps target
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyPerformancePreset();

private:
    // Registered AI actors
    UPROPERTY()
    TArray<AActor*> RegisteredActors;

    // Per-actor budget data
    TMap<AActor*, FPerf_ActorBudget> ActorBudgets;

    // Timer accumulator for LOD updates
    float LODUpdateAccumulator = 0.f;

    // FPS measurement accumulator
    float FPSAccumulator = 0.f;
    int32 FPSFrameCount = 0;

    // Singleton
    static APerf_PerformanceManager* Instance;

    // Internal helpers
    void UpdateAILODTiers();
    void UpdateFrameBudget(float DeltaTime);
    EPerf_AILODTier CalculateLODTier(float Distance) const;
    void ApplyLODTierToActor(AActor* Actor, EPerf_AILODTier NewTier, FPerf_ActorBudget& Budget);
};
