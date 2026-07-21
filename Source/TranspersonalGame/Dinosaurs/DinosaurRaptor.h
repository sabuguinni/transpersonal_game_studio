// DinosaurRaptor.h
// Performance Optimizer #04 — Pack hunter Raptor with flanking AI
// Transpersonal Game Studio — Prehistoric Survival Game

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "DinosaurRaptor.generated.h"

// Pack role enum — unique prefix Perf_ to avoid collisions
UENUM(BlueprintType)
enum class EPerf_RaptorPackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),
    Flanker     UMETA(DisplayName = "Flanker"),
    Distractor  UMETA(DisplayName = "Distractor"),
    Lone        UMETA(DisplayName = "Lone"),
};

/**
 * ADinosaurRaptor — Velociraptor pack hunter
 *
 * Pack mechanics:
 *   - Alpha calls pack to attack; flankers circle the target
 *   - Distractor draws attention while flankers strike from behind
 *   - Lone raptors are less aggressive (50% aggression modifier)
 *
 * Performance budget:
 *   - Pack tick runs at 0.25s interval (not every frame)
 *   - Flanking angle computed once per tick, cached until next tick
 *   - Max pack size = 6 (enforced in SpawnPackMember)
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ADinosaurRaptor : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ADinosaurRaptor();

    // ── Pack Configuration ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    bool bIsPackHunter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    EPerf_RaptorPackRole PackRole = EPerf_RaptorPackRole::Lone;

    /** Max members in a single pack (performance cap) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack", meta = (ClampMin = "1", ClampMax = "6"))
    int32 MaxPackSize = 4;

    /** Radius within which pack members are recruited */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackRecruitRadius = 3000.0f;

    /** Soft reference to the alpha of this raptor's pack (null if this IS the alpha) */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<ADinosaurRaptor> PackAlpha;

    /** Current pack members (only valid on Alpha) */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    TArray<TWeakObjectPtr<ADinosaurRaptor>> PackMembers;

    // ── Flanking ────────────────────────────────────────────────────────────

    /** Angle offset from target for flanking manoeuvre (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Flanking")
    float FlankingAngle = 90.0f;

    /** Flanking approach radius around target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Flanking")
    float FlankingRadius = 300.0f;

    // ── Leap Attack ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapDamage = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapImpulseForce = 900.0f;

    // ── Distract Attack ─────────────────────────────────────────────────────

    /** Damage dealt by distractor feint (low — purpose is to draw attention) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float DistractDamage = 15.0f;

    // ── Pack Tick Interval (performance) ────────────────────────────────────

    /** Seconds between pack coordination ticks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Performance", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float PackTickInterval = 0.25f;

    // ── Public API ──────────────────────────────────────────────────────────

    /** Alpha calls pack to attack a target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void CallPackToAttack(AActor* Target);

    /** Recruit nearby raptors into this pack (called on Alpha) */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void RecruitPackMembers();

    /** Execute leap attack toward target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformLeap(AActor* Target);

    /** Can this raptor leap right now? */
    UFUNCTION(BlueprintPure, Category = "Raptor|Combat")
    bool CanLeap() const;

    /** Assign a role to this raptor within the pack */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void AssignPackRole(EPerf_RaptorPackRole NewRole);

    /** Compute flanking destination around target */
    UFUNCTION(BlueprintPure, Category = "Raptor|Flanking")
    FVector ComputeFlankingDestination(AActor* Target) const;

    // ── Overrides ───────────────────────────────────────────────────────────

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Called when this raptor dies — notify pack alpha */
    virtual void OnDeath_Implementation() override;

protected:
    /** Timer handle for pack coordination tick */
    FTimerHandle PackTickHandle;

    /** Last time leap was performed */
    float LastLeapTime = -999.0f;

    /** Cached flanking destination (updated each pack tick) */
    FVector CachedFlankDestination = FVector::ZeroVector;

    /** Internal pack coordination tick */
    void PackCoordinationTick();

    /** Elect a new alpha if this raptor was the alpha */
    void ElectNewAlpha();
};
